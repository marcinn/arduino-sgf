#include "FastST7789.h"

#include "SGF/Color565.h"

namespace {
constexpr int FILL_SCREEN_STRIP_H = 4;
constexpr int FILL_SCREEN_MAX_W = 320;
constexpr int FILL_RECT_MAX_W = 60;
constexpr int FILL_RECT_MAX_H = 40;
constexpr int SWAP_TMP_MAX_PIXELS = 320 * 4;

uint16_t be16(uint16_t value) { return (uint16_t)((value << 8) | (value >> 8)); }

}  // namespace

constexpr uint8_t FastST7789::st7789Madctl(ScreenRotation rotation) {
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

constexpr FastST7789::ScreenRotation FastST7789::toScreenRotation(IScreen::Rotation rotation) {
    return static_cast<ScreenRotation>(rotation);
}

constexpr IScreen::Rotation FastST7789::toInterfaceRotation(ScreenRotation rotation) {
    return static_cast<IScreen::Rotation>(rotation);
}

FastST7789::FastST7789(IDisplayBus& bus, const PanelConfig& panel)
    : bus_(bus), panel(panel), curW(panel.width), curH(panel.height) {}

void FastST7789::setSPIFrequency(uint32_t spi_hz) { bus_.setFrequency(spi_hz); }

void FastST7789::applyBacklightLevel(uint8_t level) {
    backlightLevel = level;
    bus_.setBacklight(level);
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

void FastST7789::cmd(uint8_t command) { bus_.writeCommand(command); }

void FastST7789::data(const uint8_t* bytes, size_t size) { bus_.writeData(bytes, size); }

void FastST7789::streamBegin() { bus_.beginDataWrite(); }

void FastST7789::streamEnd() { bus_.endDataWrite(); }

FastST7789::Offset FastST7789::currentOffset() const {
    switch (currentRotation_) {
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

void FastST7789::hwReset() { bus_.hardwareReset(); }

void FastST7789::updateDimensions() {
    bool swapped = currentRotation_ == ScreenRotation::Landscape ||
                   currentRotation_ == ScreenRotation::LandscapeFlip;
    if (swapped) {
        curW = panel.height;
        curH = panel.width;
        return;
    }
    curW = panel.width;
    curH = panel.height;
}

void FastST7789::screenRotation(ScreenRotation nextRotation) {
    currentRotation_ = nextRotation;
    uint8_t madctl = (uint8_t)(st7789Madctl(nextRotation) | panel.colorOrder);
    cmd(0x36);
    data(&madctl, 1);
    updateDimensions();
}

void FastST7789::setRotation(IScreen::Rotation rotation) {
    screenRotation(toScreenRotation(rotation));
}

IScreen::Rotation FastST7789::rotation() const { return toInterfaceRotation(currentRotation_); }

bool FastST7789::scrollAxisInverted() const {
    return (st7789Madctl(currentRotation_) & MADCTL_MY) != 0;
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
    if (!bus_.begin(spi_hz)) {
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
    static uint16_t strip[FILL_SCREEN_MAX_W * FILL_SCREEN_STRIP_H];
    const bool usePixelWrite = bus_.supportsWritePixels565();
    uint16_t color = usePixelWrite ? color565 : Color565::bswap(color565);

    for (int i = 0; i < FILL_SCREEN_MAX_W * FILL_SCREEN_STRIP_H; i++) {
        strip[i] = color;
    }

    for (int y = 0; y < curH; y += FILL_SCREEN_STRIP_H) {
        int h = (y + FILL_SCREEN_STRIP_H <= curH) ? FILL_SCREEN_STRIP_H : (curH - y);
        setWindow(0, y, curW - 1, y + h - 1);
        streamBegin();
        if (usePixelWrite) {
            bus_.writePixels565(strip, (size_t)(curW * h));
        } else {
            bus_.writeDataChunk((const uint8_t*)strip, (size_t)(curW * h * 2));
        }
        streamEnd();
    }
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

    static uint16_t rectBuf[FILL_RECT_MAX_W * FILL_RECT_MAX_H];

    for (int ty = 0; ty < h; ty += FILL_RECT_MAX_H) {
        int hh = min(FILL_RECT_MAX_H, h - ty);
        for (int tx = 0; tx < w; tx += FILL_RECT_MAX_W) {
            int ww = min(FILL_RECT_MAX_W, w - tx);
            int count = ww * hh;
            for (int i = 0; i < count; i++) {
                rectBuf[i] = color565;
            }
            blit565(x0 + tx, y0 + ty, ww, hh, rectBuf);
        }
    }
}

void FastST7789::blit565(int x0, int y0, int w, int h, const uint16_t* pix) {
    if (w <= 0 || h <= 0) {
        return;
    }

    const int count = w * h;
    setWindow(x0, y0, x0 + w - 1, y0 + h - 1);
    streamBegin();
    if (bus_.supportsWritePixels565()) {
        bus_.writePixels565(pix, (size_t)count);
    } else {
        static uint16_t tmp[SWAP_TMP_MAX_PIXELS];
        int offset = 0;
        while (offset < count) {
            int chunk = min(SWAP_TMP_MAX_PIXELS, count - offset);
            for (int i = 0; i < chunk; i++) {
                tmp[i] = Color565::bswap(pix[offset + i]);
            }
            bus_.writeDataChunk((const uint8_t*)tmp, (size_t)(chunk * 2));
            offset += chunk;
        }
    }
    streamEnd();
}
