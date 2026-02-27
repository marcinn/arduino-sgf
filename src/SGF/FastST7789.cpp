#include "FastST7789.h"

#include "SGF/Color565.h"
#include "SGF/Font5x7.h"

namespace {

uint16_t be16(uint16_t value) {
  return (uint16_t)((value << 8) | (value >> 8));
}

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

void FastST7789::setSPIFrequency(uint32_t spi_hz) {
  bus_.setFrequency(spi_hz);
}

void FastST7789::setBacklight(uint8_t level) {
  backlightLevel = level;
  bus_.setBacklight(level);
}

void FastST7789::fadeBacklightTo(uint8_t targetLevel, uint32_t durationMs) {
  uint8_t startLevel = backlightLevel;
  if (durationMs == 0 || startLevel == targetLevel) {
    setBacklight(targetLevel);
    return;
  }

  uint32_t t0 = millis();
  while (true) {
    uint32_t elapsed = millis() - t0;
    if (elapsed >= durationMs) {
      break;
    }

    int32_t delta = (int32_t)targetLevel - (int32_t)startLevel;
    uint8_t current =
      (uint8_t)((int32_t)startLevel + (delta * (int32_t)elapsed) / (int32_t)durationMs);
    setBacklight(current);
    delay(1);
  }

  setBacklight(targetLevel);
}

bool FastST7789::begin(uint32_t spi_hz) {
  return begin(spi_hz, ScreenRotation::Landscape);
}

void FastST7789::cmd(uint8_t command) {
  bus_.writeCommand(command);
}

void FastST7789::data(const uint8_t* bytes, size_t size) {
  bus_.writeData(bytes, size);
}

void FastST7789::streamBegin() {
  bus_.beginDataWrite();
}

void FastST7789::streamEnd() {
  bus_.endDataWrite();
}

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

  uint16_t xs[2] = {
    be16((uint16_t)(x0 + offset.x)),
    be16((uint16_t)(x1 + offset.x))
  };
  uint16_t ys[2] = {
    be16((uint16_t)(y0 + offset.y)),
    be16((uint16_t)(y1 + offset.y))
  };

  cmd(0x2A);
  data((uint8_t*)xs, sizeof(xs));

  cmd(0x2B);
  data((uint8_t*)ys, sizeof(ys));

  cmd(0x2C);
}

void FastST7789::hwReset() {
  bus_.hardwareReset();
}

void FastST7789::updateDimensions() {
  bool swapped =
    currentRotation_ == ScreenRotation::Landscape ||
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

IScreen::Rotation FastST7789::rotation() const {
  return toInterfaceRotation(currentRotation_);
}

bool FastST7789::scrollAxisInverted() const {
  return (st7789Madctl(currentRotation_) & MADCTL_MY) != 0;
}

void FastST7789::setScrollArea(uint16_t topFixed,
                               uint16_t scrollHeight,
                               uint16_t bottomFixed) {
  uint16_t def[3] = {
    be16(topFixed),
    be16(scrollHeight),
    be16(bottomFixed)
  };
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
  setBacklight(BACKLIGHT_LEVEL_MAX);

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
  static constexpr int STRIP_H = 10;
  static uint16_t strip[320 * STRIP_H];
  const bool usePixelWrite = bus_.supportsWritePixels565();
  uint16_t color = usePixelWrite ? color565 : Color565::bswap(color565);

  for (int i = 0; i < 320 * STRIP_H; i++) {
    strip[i] = color;
  }

  for (int y = 0; y < curH; y += STRIP_H) {
    int h = (y + STRIP_H <= curH) ? STRIP_H : (curH - y);
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

  static constexpr int MAX_RW = 120;
  static constexpr int MAX_RH = 80;
  static uint16_t rectBuf[MAX_RW * MAX_RH];

  for (int ty = 0; ty < h; ty += MAX_RH) {
    int hh = min(MAX_RH, h - ty);
    for (int tx = 0; tx < w; tx += MAX_RW) {
      int ww = min(MAX_RW, w - tx);
      int count = ww * hh;
      for (int i = 0; i < count; i++) {
        rectBuf[i] = color565;
      }
      blit565(x0 + tx, y0 + ty, ww, hh, rectBuf);
    }
  }
}

void FastST7789::drawText(int x, int y, const char* text, int scale, uint16_t color565) {
  if (!text || scale <= 0) {
    return;
  }

  const int width = Font5x7::textWidth(text, scale);
  const int height = 7 * scale;
  for (int yy = 0; yy < height; yy++) {
    for (int xx = 0; xx < width; xx++) {
      if (Font5x7::textPixel(text, scale, xx, yy)) {
        fillRect565(x + xx, y + yy, 1, 1, color565);
      }
    }
  }
}

void FastST7789::drawCenteredText(int y, const char* text, int scale, uint16_t color565) {
  if (!text) {
    return;
  }
  int x = (width() - Font5x7::textWidth(text, scale)) / 2;
  drawText(x, y, text, scale, color565);
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
    static uint16_t tmp[120 * 80];
    for (int i = 0; i < count; i++) {
      tmp[i] = Color565::bswap(pix[i]);
    }
    bus_.writeDataChunk((const uint8_t*)tmp, (size_t)(count * 2));
  }
  streamEnd();
}
