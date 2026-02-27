#pragma once

#include <stdint.h>

class IScreen {
public:
  enum class Rotation : uint8_t {
    Portrait = 0,
    Landscape = 1,
    PortraitFlip = 2,
    LandscapeFlip = 3,
  };

  virtual ~IScreen() = default;

  virtual void setRotation(Rotation rotation) = 0;
  virtual Rotation rotation() const = 0;

  virtual void fillScreen565(uint16_t color565) = 0;
  virtual void fillRect565(int x0, int y0, int w, int h, uint16_t color565) = 0;

  virtual void setBacklight(uint8_t level) = 0;
  virtual uint8_t backlight() const = 0;
};
