#pragma once

#include <stdint.h>
#include <functional>

#include "FastILI9341.h"

// 1D hardware scroll helper for ILI9341 (VSCRDEF/VSCRSADD).
// The visible on-screen axis depends on the current rotation:
// - Portrait: scroll is along screen Y (up/down)
// - Landscape: scroll is along screen X (left/right)
// The API exposes a single offset on that active axis.
class HardwareScroller {
public:
  // `worldOffset` is the logical coordinate of the newly exposed strip on the
  // active scroll axis. `span` is the strip thickness in pixels.
  // Buffer layout is row-major for the strip rectangle:
  // - Portrait:  width() x span
  // - Landscape: span x height()
  using RenderStripFn = std::function<void(int32_t worldOffset, int span, uint16_t* buf)>;
  using BlitStripFn   = std::function<void(int physPos, int span, uint16_t* buf)>;

  explicit HardwareScroller(FastILI9341& gfx) : gfx(gfx) {}

  // fixedStart + scrollSpan + fixedEnd must equal the active axis length
  // (Portrait: height(), Landscape: width()).
  void configure(uint16_t topFixed, uint16_t scrollHeight, uint16_t bottomFixed);
  void configureFullScreen();

  // Sets VSCRSADD memory offset (0..scrollSpan-1).
  void resetOffset(uint16_t yOff = 0);

  // Scrolls by `delta` pixels on the active axis:
  // - Portrait:  delta>0 moves the image up (new strip appears at the bottom)
  // - Landscape: delta>0 moves the image left (new strip appears on the right)
  //
  // `buf` must contain at least:
  // - Portrait:  width() * maxStripLines pixels
  // - Landscape: maxStripLines * height() pixels
  //
  // Large deltas are split into chunks with |step| <= maxStripLines.
  // If `blitStrip` is not provided, a default full-strip blit is used.
  void scroll(int delta,
              uint16_t* buf,
              int maxStripLines,
              const RenderStripFn& renderStrip,
              const BlitStripFn& blitStrip = {});

  bool scrollsAlongY() const { return gfx.height() >= gfx.width(); }
  bool axisInverted() const { return gfx.scrollAxisInverted(); }
  uint16_t axisLength() const { return (uint16_t)(scrollsAlongY() ? gfx.height() : gfx.width()); }
  uint16_t crossLength() const { return (uint16_t)(scrollsAlongY() ? gfx.width() : gfx.height()); }

  uint16_t fixedStart() const { return topFixed_; }
  uint16_t scrollSpan() const { return scrollH_; }
  uint16_t fixedEnd() const { return bottomFixed_; }
  int32_t worldOffset() const { return worldTop_; }

  // Legacy aliases kept for compatibility with existing portrait-oriented code.
  uint16_t offset() const { return offset_; }
  int32_t worldTop() const { return worldTop_; }
  uint16_t topFixed() const { return topFixed_; }
  uint16_t scrollHeight() const { return scrollH_; }

private:
  FastILI9341& gfx;
  uint16_t topFixed_ = 0;     // fixed region at the start of the active axis
  uint16_t scrollH_ = 0;      // scroll span on the active axis
  uint16_t bottomFixed_ = 0;  // fixed region at the end of the active axis
  uint16_t offset_ = 0;       // current VSCRSADD value (mod scroll span)
  int32_t worldTop_ = 0;      // logical offset of the visible scroll-area start
};
