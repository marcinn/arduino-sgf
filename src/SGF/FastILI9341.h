#pragma once
#include <Arduino.h>

extern "C" {
  #include <zephyr/device.h>
  #include <zephyr/drivers/spi.h>
}

class FastILI9341 {
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

  // piny: CS/DC/RST/LED (LED może być -1)
  FastILI9341(int cs, int dc, int rst, int led);

  bool begin(uint32_t spi_hz);  // init z domyślną orientacją
  bool begin(uint32_t spi_hz, uint8_t madctl);
  void setSPIFrequency(uint32_t spi_hz);
  void screenRotation(uint8_t madctl);
  void screenRotation(ScreenRotation rot) { screenRotation((uint8_t)rot); }
  void setBacklight(uint8_t level);             // 0..255
  uint8_t backlight() const { return backlightLevel; }
  void setBacklightPwmMax(uint32_t pwmMax) { backlightPwmMaxValue = pwmMax ? pwmMax : 255u; }
  uint32_t backlightPwmMax() const { return backlightPwmMaxValue; }
  void fadeBacklightTo(uint8_t targetLevel, uint32_t durationMs);
  void fadeInBacklight(uint32_t durationMs) { fadeBacklightTo(255, durationMs); }
  void fadeOutBacklight(uint32_t durationMs) { fadeBacklightTo(0, durationMs); }

  int width()  const { return W; }
  int height() const { return H; }

  void fillScreen565(uint16_t color565); // color w normalnym RGB565 (nie-swapped)

  // Blit: wysyła bufor RGB565 (normalny endian) do prostokąta
  // bufor ma w*h pixeli, row-major
  void blit565(int x0, int y0, int w, int h, const uint16_t* pix);

private:
  int PIN_CS, PIN_DC, PIN_RST, PIN_LED;
  static constexpr int W = 320;
  static constexpr int H = 240;

  const struct device* spiDev = nullptr;
  struct spi_config spiCfg{};
  uint8_t backlightLevel = 255;
  uint32_t backlightPwmMaxValue = 255;

  void hwReset();
  void cmd(uint8_t c);
  void data(const uint8_t* d, size_t n);
  void setWindow(int x0,int y0,int x1,int y1);

  void streamBegin();
  void streamEnd();
};
