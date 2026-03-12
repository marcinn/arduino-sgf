#include "FastILI9341.h"

#include "Color565.h"

uint8_t FastILI9341::toMadctl(ScreenRotation rotation) {
    switch (rotation) {
        case ScreenRotation::Portrait:
            return 0x48u;
        case ScreenRotation::Landscape:
            return 0xE8u;
        case ScreenRotation::PortraitFlip:
            return 0x88u;
        case ScreenRotation::LandscapeFlip:
        default:
            return 0x28u;
    }
}

ScreenRotation FastILI9341::toInterfaceRotation(uint8_t madctl) {
    switch (madctl) {
        case 0x48u:
            return ScreenRotation::Portrait;
        case 0xE8u:
            return ScreenRotation::Landscape;
        case 0x88u:
            return ScreenRotation::PortraitFlip;
        case 0x28u:
        default:
            return ScreenRotation::LandscapeFlip;
    }
}

FastILI9341::FastILI9341(IDisplayBus& bus) : bus(bus) {}

void FastILI9341::applyBacklightLevel(uint8_t level) {
    backlightLevel = level;
    bus.setBacklight(level);
}

void FastILI9341::setBacklight(uint8_t level) {
    backlightFade.stop();
    applyBacklightLevel(level);
}

void FastILI9341::setRotation(ScreenRotation rotation) { screenRotation(toMadctl(rotation)); }

ScreenRotation FastILI9341::rotation() const { return toInterfaceRotation(rotationMadctl); }

void FastILI9341::fadeBacklightTo(uint8_t targetLevel, uint32_t durationMs) {
    uint8_t startLevel = backlightLevel;
    if (durationMs == 0 || startLevel == targetLevel) {
        setBacklight(targetLevel);
        return;
    }

    backlightFade.start(startLevel, targetLevel, millis(), durationMs);
}

void FastILI9341::tickEffects() { updateBacklightFade(); }

void FastILI9341::updateBacklightFade() {
    if (!backlightFade.active) {
        return;
    }

    uint32_t now = millis();
    if (backlightFade.isComplete(now)) {
        applyBacklightLevel(backlightFade.targetLevel);
        backlightFade.stop();
        return;
    }

    applyBacklightLevel(backlightFade.levelAt(now));
}

bool FastILI9341::begin(uint32_t spi_hz) {
    return begin(spi_hz, toMadctl(ScreenRotation::Landscape));
}

static inline uint16_t be16(uint16_t v) { return (uint16_t)((v << 8) | (v >> 8)); }

void FastILI9341::cmd(uint8_t c) { bus.writeCommand(c); }

void FastILI9341::data(const uint8_t* d, size_t n) { bus.writeData(d, n); }

void FastILI9341::streamBegin() { bus.beginDataWrite(); }
void FastILI9341::streamEnd() { bus.endDataWrite(); }

void FastILI9341::setWindow(int x0, int y0, int x1, int y1) {
    cmd(0x2A);
    uint16_t xd[2] = {be16((uint16_t)x0), be16((uint16_t)x1)};
    data((uint8_t*)xd, 4);

    cmd(0x2B);
    uint16_t yd[2] = {be16((uint16_t)y0), be16((uint16_t)y1)};
    data((uint8_t*)yd, 4);

    cmd(0x2C);
}

void FastILI9341::setScrollArea(uint16_t topFixed, uint16_t scrollHeight, uint16_t bottomFixed) {
    // VSCRDEF (0x33): 3 words: TFA, VSA, BFA
    uint16_t def[3] = {be16(topFixed), be16(scrollHeight), be16(bottomFixed)};
    cmd(0x33);
    data((uint8_t*)def, 6);
}

void FastILI9341::scrollTo(uint16_t yOff) {
    // VSCRSADD (0x37): 1 word, scroll start address (0..VSA-1)
    uint16_t off = be16(yOff);
    cmd(0x37);
    data((uint8_t*)&off, 2);
}

void FastILI9341::hwReset() { bus.hardwareReset(); }

void FastILI9341::screenRotation(uint8_t madctl) {
    rotationMadctl = madctl;
    cmd(0x36);  // MADCTL
    data(&madctl, 1);
    updateDimensions(madctl);
}

bool FastILI9341::begin(uint32_t spi_hz, uint8_t madctl) {
    if (!bus.begin(spi_hz)) return false;
    applyBacklightLevel(backlightLevel);

    hwReset();

    cmd(0x01);
    delay(150);  // SWRESET
    cmd(0x11);
    delay(120);  // SLPOUT

    cmd(0x3A);  // COLMOD
    {
        uint8_t col = 0x55;
        data(&col, 1);
    }
    delay(10);

    screenRotation(madctl);
    delay(10);

    cmd(0x29);
    delay(20);  // DISPON
    updateDimensions(madctl);
    return true;
}

void FastILI9341::updateDimensions(uint8_t madctl) {
    // Portrait variants (with MV cleared) should expose H x W logical size.
    // Landscape variants (with MV set) expose W x H.
    bool portrait = (madctl == toMadctl(ScreenRotation::Portrait)) ||
                    (madctl == toMadctl(ScreenRotation::PortraitFlip));
    if (portrait) {
        curW = H;
        curH = W;
    } else {
        curW = W;
        curH = H;
    }
}

void FastILI9341::fillScreen565(uint16_t color565) {
    uint16_t color = Color565::bswap(color565);

    setWindow(0, 0, curW - 1, curH - 1);
    streamBegin();
    for (int i = 0; i < curW * curH; i++) {
        bus.writeDataChunk((const uint8_t*)&color, sizeof(color));
    }
    streamEnd();
}

void FastILI9341::fillRect565(int x0, int y0, int w, int h, uint16_t color565) {
    if (w <= 0 || h <= 0) {
        return;
    }

    if (x0 < 0) {
        w += x0;
        x0 = 0;
    }
    if (y0 < 0) {
        h += y0;
        y0 = 0;
    }
    if (x0 >= curW || y0 >= curH) {
        return;
    }
    if (x0 + w > curW) {
        w = curW - x0;
    }
    if (y0 + h > curH) {
        h = curH - y0;
    }
    if (w <= 0 || h <= 0) {
        return;
    }

    uint16_t color = Color565::bswap(color565);
    setWindow(x0, y0, x0 + w - 1, y0 + h - 1);
    streamBegin();
    for (int i = 0; i < w * h; i++) {
        bus.writeDataChunk((const uint8_t*)&color, sizeof(color));
    }
    streamEnd();
}

void FastILI9341::blit565(int x0, int y0, int w, int h, const uint16_t* pix) {
    if (w <= 0 || h <= 0) {
        return;
    }

    setWindow(x0, y0, x0 + w - 1, y0 + h - 1);
    streamBegin();
    for (int i = 0; i < w * h; i++) {
        uint16_t swapped = Color565::bswap(pix[i]);
        bus.writeDataChunk((const uint8_t*)&swapped, sizeof(swapped));
    }
    streamEnd();
}

void FastILI9341::beginBlit565Stream(int x0, int y0, int w, int h) {
    if (w <= 0 || h <= 0) {
        return;
    }

    setWindow(x0, y0, x0 + w - 1, y0 + h - 1);
    streamBegin();
}

void FastILI9341::writeBlit565StreamChunk(const uint16_t* pix, size_t count) {
    if (!pix || count == 0u) {
        return;
    }

    for (size_t i = 0; i < count; i++) {
        uint16_t swapped = Color565::bswap(pix[i]);
        bus.writeDataChunk((const uint8_t*)&swapped, sizeof(swapped));
    }
}

void FastILI9341::endBlit565Stream() { streamEnd(); }
