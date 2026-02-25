#pragma once
#include <Arduino.h>
#include "IRenderTarget.h"

extern "C" {
  #include <zephyr/device.h>
  #include <zephyr/drivers/spi.h>
}

class FastILI9341 : public IRenderTarget {
public:
  enum class ScreenRotation : uint8_t {
    Landscape      = 0xE8,
    Portrait       = 0x48,
    LandscapeFlip  = 0x28,
    PortraitFlip   = 0x88,
  };
  using Rotation = ScreenRotation;  // backward-compatible alias

  static constexpr uint8_t MADCTL_MY  = 0x80;
  static constexpr uint8_t MADCTL_MX  = 0x40;
  static constexpr uint8_t MADCTL_MV  = 0x20;
  static constexpr uint8_t MADCTL_ML  = 0x10;
  static constexpr uint8_t MADCTL_BGR = 0x08;
  static constexpr uint8_t MADCTL_MH  = 0x04;
  static constexpr uint8_t BACKLIGHT_LEVEL_MIN = 0u;
  static constexpr uint8_t BACKLIGHT_LEVEL_MAX = 255u;

  // piny: CS/DC/RST/LED (LED może być -1)
  FastILI9341(int cs, int dc, int rst, int led);

  bool begin(uint32_t spi_hz);  // init z domyślną orientacją
  bool begin(uint32_t spi_hz, uint8_t madctl);
  void setSPIFrequency(uint32_t spi_hz);
  void screenRotation(uint8_t madctl);
  void screenRotation(ScreenRotation rot) { screenRotation((uint8_t)rot); }
  uint8_t rotationMadctl() const { return rotationMadctl_; }
  // The ILI9341 vertical-scroll axis is mirrored when MADCTL_MY is set.
  bool scrollAxisInverted() const { return (rotationMadctl_ & MADCTL_MY) != 0; }
  void setBacklight(uint8_t level);  // normalized brightness 0..BACKLIGHT_LEVEL_MAX
  uint8_t backlight() const { return backlightLevel; }
  void setBacklightPwmMax(uint32_t pwmMax) {
    backlightPwmMaxValue = pwmMax ? pwmMax : (uint32_t)BACKLIGHT_LEVEL_MAX;
  }
  uint32_t backlightPwmMax() const { return backlightPwmMaxValue; }
  void fadeBacklightTo(uint8_t targetLevel, uint32_t durationMs);
  void fadeInBacklight(uint32_t durationMs) { fadeBacklightTo(BACKLIGHT_LEVEL_MAX, durationMs); }
  void fadeOutBacklight(uint32_t durationMs) { fadeBacklightTo(BACKLIGHT_LEVEL_MIN, durationMs); }

  int width() const override { return curW; }
  int height() const override { return curH; }

  void fillScreen565(uint16_t color565); // color w normalnym RGB565 (nie-swapped)
  void fillRect565(int x0, int y0, int w, int h, uint16_t color565);
  void drawText(int x, int y, const char* text, int scale, uint16_t color565);
  void drawCenteredText(int y, const char* text, int scale, uint16_t color565);

  // Blit: wysyła bufor RGB565 (normalny endian) do prostokąta
  // bufor ma w*h pixeli, row-major
  void blit565(int x0, int y0, int w, int h, const uint16_t* pix) override;

  // Sprzętowy vertical scroll ILI9341
  // top/height/bottom muszą sumować się do height() w aktualnej orientacji
  void setScrollArea(uint16_t topFixed, uint16_t scrollHeight, uint16_t bottomFixed);
  // yOff zawija się mod height(); 0 = topFixed w górnym rogu
  void scrollTo(uint16_t yOff);

private:
  int PIN_CS, PIN_DC, PIN_RST, PIN_LED;
  static constexpr int W = 320;
  static constexpr int H = 240;
  int curW = W;
  int curH = H;

  const struct device* spiDev = nullptr;
  struct spi_config spiCfg{};
  uint8_t backlightLevel = BACKLIGHT_LEVEL_MAX;
  uint32_t backlightPwmMaxValue = BACKLIGHT_LEVEL_MAX;
  uint8_t rotationMadctl_ = (uint8_t)ScreenRotation::Landscape;

  void updateDimensions(uint8_t madctl);
  void hwReset();
  void cmd(uint8_t c);
  void data(const uint8_t* d, size_t n);
  void setWindow(int x0,int y0,int x1,int y1);

  void streamBegin();
  void streamEnd();
};
