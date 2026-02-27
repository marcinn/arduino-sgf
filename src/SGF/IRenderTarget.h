#pragma once

#include <stdint.h>

class IRenderTarget {
public:
  virtual ~IRenderTarget() = default;
  virtual int width() const = 0;
  virtual int height() const = 0;
  virtual void blit565(int x0, int y0, int w, int h, const uint16_t* pix) = 0;
  virtual void tickEffects() {}
  virtual bool supportsHardwareScroll() const { return false; }
  virtual void setScrollArea(uint16_t fixedStart, uint16_t scrollSpan, uint16_t fixedEnd) {
    (void)fixedStart;
    (void)scrollSpan;
    (void)fixedEnd;
  }
  virtual void scrollTo(uint16_t offset) {
    (void)offset;
  }
  virtual bool scrollAxisInverted() const { return false; }
};
