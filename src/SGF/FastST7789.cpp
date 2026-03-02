#include "FastST7789.h"

#include "Color565.h"

namespace {
uint16_t be16(uint16_t value) { return (uint16_t)((value << 8) | (value >> 8)); }

}  // namespace

uint8_t FastST7789::st7789Madctl(ScreenRotation rotation) {
    switch (rotation) {
        case ScreenRotation::Portrait:
            return (uint8_t)(MADCTL_MX | MADCTL_MY);
        case ScreenRotation::Landscape:
            return (uint8_t)(MADCTL_MY | MADCTL_MV);
        case ScreenRotation::PortraitFlip:
            return 0;
        case ScreenRotation::LandscapeFlip:
        default:
            return (uint8_t)(MADCTL_MX | MADCTL_MV);
    }
}

FastST7789::FastST7789(IDisplayBus& bus, const PanelConfig& panel)
    : bus(bus), panel(panel), curW(panel.width), curH(panel.height) {}

void FastST7789::applyBacklightLevel(uint8_t level) {
    backlightLevel = level;
    bus.setBacklight(level);
}

void FastST7789::setBacklight(uint8_t level) {
    backlightFade.stop();
    applyBacklightLevel(level);
}

void FastST7789::fadeBacklightTo(uint8_t targetLevel, uint32_t durationMs) {
    uint8_t startLevel = backlightLevel;
    if (durationMs == 0 || startLevel == targetLevel) {
        setBacklight(targetLevel);
        return;
    }

    backlightFade.start(startLevel, targetLevel, millis(), durationMs);
}

void FastST7789::tickEffects() { updateBacklightFade(); }

void FastST7789::updateBacklightFade() {
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

bool FastST7789::begin(uint32_t spi_hz) { return begin(spi_hz, ScreenRotation::Landscape); }

void FastST7789::cmd(uint8_t command) { bus.writeCommand(command); }

void FastST7789::data(const uint8_t* bytes, size_t size) { bus.writeData(bytes, size); }

void FastST7789::streamBegin() { bus.beginDataWrite(); }

void FastST7789::streamEnd() { bus.endDataWrite(); }

FastST7789::Offset FastST7789::currentOffset() const {
    switch (currentRotation) {
        case ScreenRotation::Portrait:
            return panel.portrait;
        case ScreenRotation::Landscape:
            return panel.landscape;
        case ScreenRotation::PortraitFlip:
            return panel.portraitFlip;
        case ScreenRotation::LandscapeFlip:
        default:
            return panel.landscapeFlip;
    }
}

void FastST7789::setWindow(int x0, int y0, int x1, int y1) {
    Offset offset = currentOffset();

    uint16_t xs[2] = {be16((uint16_t)(x0 + offset.x)), be16((uint16_t)(x1 + offset.x))};
    uint16_t ys[2] = {be16((uint16_t)(y0 + offset.y)), be16((uint16_t)(y1 + offset.y))};

    cmd(0x2A);
    data((uint8_t*)xs, sizeof(xs));

    cmd(0x2B);
    data((uint8_t*)ys, sizeof(ys));

    cmd(0x2C);
}

void FastST7789::hwReset() { bus.hardwareReset(); }

void FastST7789::updateDimensions() {
    bool swapped = currentRotation == ScreenRotation::Landscape ||
                   currentRotation == ScreenRotation::LandscapeFlip;
    if (swapped) {
        curW = panel.height;
        curH = panel.width;
        return;
    }
    curW = panel.width;
    curH = panel.height;
}

void FastST7789::screenRotation(ScreenRotation nextRotation) {
    currentRotation = nextRotation;
    uint8_t madctl = (uint8_t)(st7789Madctl(nextRotation) | panel.colorOrder);
    cmd(0x36);
    data(&madctl, 1);
    updateDimensions();
}

void FastST7789::setRotation(ScreenRotation rotation) {
    screenRotation(rotation);
}

ScreenRotation FastST7789::rotation() const { return currentRotation; }

bool FastST7789::scrollAxisInverted() const {
    return (st7789Madctl(currentRotation) & MADCTL_MY) != 0;
}

void FastST7789::setScrollArea(uint16_t topFixed, uint16_t scrollHeight, uint16_t bottomFixed) {
    uint16_t def[3] = {be16(topFixed), be16(scrollHeight), be16(bottomFixed)};
    cmd(0x33);
    data((uint8_t*)def, sizeof(def));
}

void FastST7789::scrollTo(uint16_t yOffset) {
    uint16_t offset = be16(yOffset);
    cmd(0x37);
    data((uint8_t*)&offset, sizeof(offset));
}

bool FastST7789::begin(uint32_t spi_hz, ScreenRotation initialRotation) {
    if (!bus.begin(spi_hz)) {
        return false;
    }
    applyBacklightLevel(backlightLevel);

    hwReset();

    cmd(0x01);
    delay(150);

    cmd(0x11);
    delay(120);

    cmd(0x3A);
    {
        uint8_t format = 0x55;
        data(&format, 1);
    }
    delay(10);

    if (panel.invertColors) {
        cmd(0x21);
    } else {
        cmd(0x20);
    }
    delay(10);

    cmd(0x13);
    delay(10);

    screenRotation(initialRotation);
    delay(10);

    cmd(0x29);
    delay(20);
    updateDimensions();
    return true;
}

void FastST7789::fillScreen565(uint16_t color565) {
    const bool usePixelWrite = bus.supportsWritePixels565();
    uint16_t color = usePixelWrite ? color565 : Color565::bswap(color565);

    setWindow(0, 0, curW - 1, curH - 1);
    streamBegin();
    for (int i = 0; i < curW * curH; i++) {
        if (usePixelWrite) {
            bus.writePixels565(&color, 1u);
        } else {
            bus.writeDataChunk((const uint8_t*)&color, sizeof(color));
        }
    }
    streamEnd();
}

void FastST7789::fillRect565(int x0, int y0, int w, int h, uint16_t color565) {
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

    const bool usePixelWrite = bus.supportsWritePixels565();
    uint16_t color = usePixelWrite ? color565 : Color565::bswap(color565);

    setWindow(x0, y0, x0 + w - 1, y0 + h - 1);
    streamBegin();
    for (int i = 0; i < w * h; i++) {
        if (usePixelWrite) {
            bus.writePixels565(&color, 1u);
        } else {
            bus.writeDataChunk((const uint8_t*)&color, sizeof(color));
        }
    }
    streamEnd();
}

void FastST7789::blit565(int x0, int y0, int w, int h, const uint16_t* pix) {
    if (w <= 0 || h <= 0) {
        return;
    }

    const int count = w * h;
    setWindow(x0, y0, x0 + w - 1, y0 + h - 1);
    streamBegin();
    if (bus.supportsWritePixels565()) {
        bus.writePixels565(pix, (size_t)count);
    } else {
        for (int i = 0; i < count; i++) {
            uint16_t swapped = Color565::bswap(pix[i]);
            bus.writeDataChunk((const uint8_t*)&swapped, sizeof(swapped));
        }
    }
    streamEnd();
}
