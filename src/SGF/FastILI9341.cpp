#include "FastILI9341.h"
#include "SGF/Color565.h"
#include "SGF/Font5x7.h"

FastILI9341::FastILI9341(int cs, int dc, int rst, int led)
  : PIN_CS(cs), PIN_DC(dc), PIN_RST(rst), PIN_LED(led) {}

void FastILI9341::setSPIFrequency(uint32_t spi_hz) {
  spiCfg.frequency = spi_hz;
}

void FastILI9341::setBacklight(uint8_t level) {
  backlightLevel = level;
  if (PIN_LED < 0) return;

  uint32_t pwm =
    ((uint32_t)level * backlightPwmMaxValue + (BACKLIGHT_LEVEL_MAX / 2u)) / BACKLIGHT_LEVEL_MAX;

  if (pwm == 0u) {
    digitalWrite(PIN_LED, LOW);
    return;
  }
  if (pwm >= backlightPwmMaxValue) {
    digitalWrite(PIN_LED, HIGH);
    return;
  }
  analogWrite(PIN_LED, (int)pwm);
}

void FastILI9341::fadeBacklightTo(uint8_t targetLevel, uint32_t durationMs) {
  uint8_t startLevel = backlightLevel;
  if (durationMs == 0 || startLevel == targetLevel) {
    setBacklight(targetLevel);
    return;
  }

  uint32_t t0 = millis();
  while (true) {
    uint32_t elapsed = millis() - t0;
    if (elapsed >= durationMs) break;

    int32_t dv = (int32_t)targetLevel - (int32_t)startLevel;
    uint8_t cur = (uint8_t)((int32_t)startLevel + (dv * (int32_t)elapsed) / (int32_t)durationMs);
    setBacklight(cur);
    delay(1);
  }

  setBacklight(targetLevel);
}

bool FastILI9341::begin(uint32_t spi_hz) {
  return begin(spi_hz, (uint8_t)ScreenRotation::Landscape);
}

static inline uint16_t be16(uint16_t v) {
  return (uint16_t)((v << 8) | (v >> 8));
}

void FastILI9341::cmd(uint8_t c) {
  digitalWrite(PIN_DC, LOW);
  digitalWrite(PIN_CS, LOW);
  spi_buf b{ .buf = (void*)&c, .len = 1 };
  spi_buf_set s{ .buffers = &b, .count = 1 };
  (void)spi_write(spiDev, &spiCfg, &s);
  digitalWrite(PIN_CS, HIGH);
}

void FastILI9341::data(const uint8_t* d, size_t n) {
  digitalWrite(PIN_DC, HIGH);
  digitalWrite(PIN_CS, LOW);
  spi_buf b{ .buf = (void*)d, .len = (uint32_t)n };
  spi_buf_set s{ .buffers = &b, .count = 1 };
  (void)spi_write(spiDev, &spiCfg, &s);
  digitalWrite(PIN_CS, HIGH);
}

void FastILI9341::streamBegin() {
  digitalWrite(PIN_DC, HIGH);
  digitalWrite(PIN_CS, LOW);
}
void FastILI9341::streamEnd() {
  digitalWrite(PIN_CS, HIGH);
}

void FastILI9341::setWindow(int x0, int y0, int x1, int y1) {
  cmd(0x2A);
  uint16_t xd[2] = { be16((uint16_t)x0), be16((uint16_t)x1) };
  data((uint8_t*)xd, 4);

  cmd(0x2B);
  uint16_t yd[2] = { be16((uint16_t)y0), be16((uint16_t)y1) };
  data((uint8_t*)yd, 4);

  cmd(0x2C);
}

void FastILI9341::setScrollArea(uint16_t topFixed, uint16_t scrollHeight, uint16_t bottomFixed) {
  // VSCRDEF (0x33): 3 words: TFA, VSA, BFA
  uint16_t def[3] = { be16(topFixed), be16(scrollHeight), be16(bottomFixed) };
  cmd(0x33);
  data((uint8_t*)def, 6);
}

void FastILI9341::scrollTo(uint16_t yOff) {
  // VSCRSADD (0x37): 1 word, scroll start address (0..VSA-1)
  uint16_t off = be16(yOff);
  cmd(0x37);
  data((uint8_t*)&off, 2);
}

void FastILI9341::hwReset() {
  if (PIN_RST < 0) return;
  digitalWrite(PIN_RST, HIGH);
  delay(5);
  digitalWrite(PIN_RST, LOW);
  delay(20);
  digitalWrite(PIN_RST, HIGH);
  delay(120);
}

void FastILI9341::screenRotation(uint8_t madctl) {
  rotationMadctl_ = madctl;
  cmd(0x36);  // MADCTL
  data(&madctl, 1);
  updateDimensions(madctl);
}

bool FastILI9341::begin(uint32_t spi_hz, uint8_t madctl) {
  pinMode(PIN_CS, OUTPUT);
  pinMode(PIN_DC, OUTPUT);
  if (PIN_RST >= 0) pinMode(PIN_RST, OUTPUT);
  if (PIN_LED >= 0) {
    pinMode(PIN_LED, OUTPUT);
    setBacklight(BACKLIGHT_LEVEL_MAX);
  }

  digitalWrite(PIN_CS, HIGH);
  digitalWrite(PIN_DC, HIGH);
  if (PIN_RST >= 0) digitalWrite(PIN_RST, HIGH);

  // UNO Q (Zephyr core): use spi2 as in the existing board setup.
  spiDev = DEVICE_DT_GET(DT_NODELABEL(spi2));
  if (!spiDev || !device_is_ready(spiDev)) return false;

  spiCfg.frequency = spi_hz;
  spiCfg.operation = SPI_OP_MODE_MASTER | SPI_WORD_SET(8) | SPI_TRANSFER_MSB;
  spiCfg.slave = 0;
  spiCfg.cs = spi_cs_control{};

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
  bool portrait = (madctl == (uint8_t)ScreenRotation::Portrait) ||
                  (madctl == (uint8_t)ScreenRotation::PortraitFlip);
  if (portrait) {
    curW = H;
    curH = W;
  } else {
    curW = W;
    curH = H;
  }
}

void FastILI9341::fillScreen565(uint16_t color565) {
  // Stream byte-swapped RGB565 (ILI9341 expects big-endian words).
  uint16_t c = Color565::bswap(color565);

  // Temporary strip buffer: maxWidth * 10 rows.
  static constexpr int STRIP_H = 10;
  static uint16_t strip[W * STRIP_H];

  for (int i = 0; i < W * STRIP_H; i++) strip[i] = c;

  for (int y = 0; y < curH; y += STRIP_H) {
    int h = (y + STRIP_H <= curH) ? STRIP_H : (curH - y);
    setWindow(0, y, curW - 1, y + h - 1);
    streamBegin();
    spi_buf b{ .buf = strip, .len = (uint32_t)(curW * h * 2) };
    spi_buf_set s{ .buffers = &b, .count = 1 };
    (void)spi_write(spiDev, &spiCfg, &s);
    streamEnd();
  }
}

void FastILI9341::fillRect565(int x0, int y0, int w, int h, uint16_t color565) {
  if (w <= 0 || h <= 0) return;

  if (x0 < 0) {
    w += x0;
    x0 = 0;
  }
  if (y0 < 0) {
    h += y0;
    y0 = 0;
  }
  if (x0 >= curW || y0 >= curH) return;
  if (x0 + w > curW) w = curW - x0;
  if (y0 + h > curH) h = curH - y0;
  if (w <= 0 || h <= 0) return;

  static constexpr int MAX_RW = 120;
  static constexpr int MAX_RH = 80;
  static uint16_t rectBuf[MAX_RW * MAX_RH];

  for (int ty = 0; ty < h; ty += MAX_RH) {
    int hh = min(MAX_RH, h - ty);
    for (int tx = 0; tx < w; tx += MAX_RW) {
      int ww = min(MAX_RW, w - tx);
      int n = ww * hh;
      for (int i = 0; i < n; i++) rectBuf[i] = color565;
      blit565(x0 + tx, y0 + ty, ww, hh, rectBuf);
    }
  }
}

void FastILI9341::drawText(int x, int y, const char* text, int scale, uint16_t color565) {
  if (!text || scale <= 0) return;

  const int w = Font5x7::textWidth(text, scale);
  const int h = 7 * scale;
  for (int yy = 0; yy < h; yy++) {
    for (int xx = 0; xx < w; xx++) {
      if (Font5x7::textPixel(text, scale, xx, yy)) {
        fillRect565(x + xx, y + yy, 1, 1, color565);
      }
    }
  }
}

void FastILI9341::drawCenteredText(int y, const char* text, int scale, uint16_t color565) {
  if (!text) return;
  int x = (width() - Font5x7::textWidth(text, scale)) / 2;
  drawText(x, y, text, scale, color565);
}

void FastILI9341::blit565(int x0, int y0, int w, int h, const uint16_t* pix) {
  if (w <= 0 || h <= 0) return;

  // ILI9341 expects big-endian words. Convert into a temporary swapped buffer.
  // Maximum size is constrained by caller-side dirty/tile dimensions.
  static uint16_t tmp[120 * 80];  // Keep in sync with typical region/tile limits.
  const int n = w * h;
  for (int i = 0; i < n; i++) tmp[i] = Color565::bswap(pix[i]);

  setWindow(x0, y0, x0 + w - 1, y0 + h - 1);
  streamBegin();
  spi_buf b{ .buf = tmp, .len = (uint32_t)(n * 2) };
  spi_buf_set s{ .buffers = &b, .count = 1 };
  (void)spi_write(spiDev, &spiCfg, &s);
  streamEnd();
}
