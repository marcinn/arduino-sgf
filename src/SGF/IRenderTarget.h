#pragma once

#include <stdint.h>

class IRenderTarget {
public:
  virtual ~IRenderTarget() = default;
  virtual int width() const = 0;
  virtual int height() const = 0;
  virtual void blit565(int x0, int y0, int w, int h, const uint16_t* pix) = 0;
};
