#pragma once

#include <Arduino.h>

#include "BacklightFade.h"
#include "IDisplayBus.h"
#include "IRenderTarget.h"
#include "IScreen.h"

class FastILI9341 : public IRenderTarget, public IScreen {
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

  explicit FastILI9341(IDisplayBus& bus);

  bool begin(uint32_t spi_hz);  // Init with the default orientation.
  bool begin(uint32_t spi_hz, uint8_t madctl);
  void setSPIFrequency(uint32_t spi_hz);
  void screenRotation(uint8_t madctl);
  void screenRotation(ScreenRotation rot) { screenRotation((uint8_t)rot); }
  void setRotation(IScreen::Rotation rotation) override;
  IScreen::Rotation rotation() const override;
  bool supportsHardwareScroll() const override { return true; }
  // The ILI9341 vertical-scroll axis is mirrored when MADCTL_MY is set.
  bool scrollAxisInverted() const override { return (rotationMadctl_ & MADCTL_MY) != 0; }
  void setBacklight(uint8_t level) override;  // normalized brightness 0..BACKLIGHT_LEVEL_MAX
  uint8_t backlight() const override { return backlightLevel; }
  void fadeBacklightTo(uint8_t targetLevel, uint32_t durationMs);
  void fadeInBacklight(uint32_t durationMs) { fadeBacklightTo(BACKLIGHT_LEVEL_MAX, durationMs); }
  void fadeOutBacklight(uint32_t durationMs) { fadeBacklightTo(BACKLIGHT_LEVEL_MIN, durationMs); }
  void tickEffects() override;

  int width() const override { return curW; }
  int height() const override { return curH; }

  void fillScreen565(uint16_t color565) override;  // Color in native RGB565 (not byte-swapped).
  void fillRect565(int x0, int y0, int w, int h, uint16_t color565) override;
  void drawText(int x, int y, const char* text, int scale, uint16_t color565);

  // Blit a row-major RGB565 buffer (native-endian) into a rectangle.
  void blit565(int x0, int y0, int w, int h, const uint16_t* pix) override;

  // ILI9341 hardware vertical scroll.
  // top + height + bottom must equal height() in the current orientation.
  void setScrollArea(uint16_t topFixed, uint16_t scrollHeight, uint16_t bottomFixed) override;
  // yOff wraps mod height(); 0 means topFixed is visible at the top edge.
  void scrollTo(uint16_t yOff) override;

private:
  IDisplayBus& bus_;
  static constexpr int W = 320;
  static constexpr int H = 240;
  int curW = W;
  int curH = H;

  uint8_t backlightLevel = BACKLIGHT_LEVEL_MAX;
  uint8_t rotationMadctl_ = (uint8_t)ScreenRotation::Landscape;
  BacklightFade backlightFade;

  static constexpr uint8_t toMadctl(IScreen::Rotation rotation);
  static constexpr IScreen::Rotation toInterfaceRotation(uint8_t madctl);
  void applyBacklightLevel(uint8_t level);
  void updateBacklightFade();
  void updateDimensions(uint8_t madctl);
  void hwReset();
  void cmd(uint8_t c);
  void data(const uint8_t* d, size_t n);
  void setWindow(int x0,int y0,int x1,int y1);

  void streamBegin();
  void streamEnd();
};
