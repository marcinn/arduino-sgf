#pragma once

#include <Arduino.h>

#include "BacklightFade.h"
#include "IDisplayBus.h"
#include "IRenderTarget.h"
#include "IScreen.h"

class FastST7789 : public IRenderTarget, public IScreen {
public:
  enum class ScreenRotation : uint8_t {
    Portrait = 0,
    Landscape = 1,
    PortraitFlip = 2,
    LandscapeFlip = 3,
  };
  using Rotation = ScreenRotation;

  static constexpr uint8_t MADCTL_MY = 0x80;
  static constexpr uint8_t MADCTL_MX = 0x40;
  static constexpr uint8_t MADCTL_MV = 0x20;
  static constexpr uint8_t MADCTL_BGR = 0x08;
  static constexpr uint8_t BACKLIGHT_LEVEL_MIN = 0u;
  static constexpr uint8_t BACKLIGHT_LEVEL_MAX = 255u;

  struct Offset {
    uint16_t x;
    uint16_t y;
  };

  struct PanelConfig {
    uint16_t width;
    uint16_t height;
    Offset portrait;
    Offset landscape;
    Offset portraitFlip;
    Offset landscapeFlip;
    uint8_t colorOrder;
    bool invertColors;
  };

  static constexpr Offset offset(uint16_t x, uint16_t y) {
    return Offset{x, y};
  }

  static constexpr PanelConfig makePanelConfig(
    uint16_t width,
    uint16_t height,
    Offset portrait = Offset{0, 0},
    Offset landscape = Offset{0, 0},
    Offset portraitFlip = Offset{0, 0},
    Offset landscapeFlip = Offset{0, 0},
    uint8_t colorOrder = MADCTL_BGR,
    bool invertColors = true
  ) {
    return PanelConfig{
      width,
      height,
      portrait,
      landscape,
      portraitFlip,
      landscapeFlip,
      colorOrder,
      invertColors
    };
  }

  FastST7789(IDisplayBus& bus, const PanelConfig& panel);

  bool begin(uint32_t spi_hz);
  bool begin(uint32_t spi_hz, ScreenRotation rotation);
  void setSPIFrequency(uint32_t spi_hz);
  void screenRotation(ScreenRotation rotation);
  void setRotation(IScreen::Rotation rotation) override;
  IScreen::Rotation rotation() const override;
  bool supportsHardwareScroll() const override { return true; }
  bool scrollAxisInverted() const override;
  void setBacklight(uint8_t level) override;
  uint8_t backlight() const override { return backlightLevel; }
  void fadeBacklightTo(uint8_t targetLevel, uint32_t durationMs);
  void fadeInBacklight(uint32_t durationMs) { fadeBacklightTo(BACKLIGHT_LEVEL_MAX, durationMs); }
  void fadeOutBacklight(uint32_t durationMs) { fadeBacklightTo(BACKLIGHT_LEVEL_MIN, durationMs); }
  void tickEffects() override;

  int width() const override { return curW; }
  int height() const override { return curH; }

  void fillScreen565(uint16_t color565) override;
  void fillRect565(int x0, int y0, int w, int h, uint16_t color565) override;
  void drawText(int x, int y, const char* text, int scale, uint16_t color565);
  void blit565(int x0, int y0, int w, int h, const uint16_t* pix) override;
  void setScrollArea(uint16_t topFixed, uint16_t scrollHeight, uint16_t bottomFixed) override;
  void scrollTo(uint16_t yOffset) override;

private:
  IDisplayBus& bus_;
  PanelConfig panel;
  int curW = 0;
  int curH = 0;
  uint8_t backlightLevel = BACKLIGHT_LEVEL_MAX;
  ScreenRotation currentRotation_ = ScreenRotation::Landscape;
  BacklightFade backlightFade;

  static constexpr uint8_t st7789Madctl(ScreenRotation rotation);
  static constexpr ScreenRotation toScreenRotation(IScreen::Rotation rotation);
  static constexpr IScreen::Rotation toInterfaceRotation(ScreenRotation rotation);
  void applyBacklightLevel(uint8_t level);
  void updateBacklightFade();
  void updateDimensions();
  Offset currentOffset() const;
  void hwReset();
  void cmd(uint8_t command);
  void data(const uint8_t* bytes, size_t size);
  void setWindow(int x0, int y0, int x1, int y1);
  void streamBegin();
  void streamEnd();
};
