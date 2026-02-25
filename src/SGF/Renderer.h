#pragma once

#include <functional>
#include <stdint.h>

#include "DirtyRects.h"
#include "Sprites.h"
#include "TileFlusher.h"
#include "Scroller.h"

// Renderer facade that combines ILI9341 hardware scroll, background redraw,
// sprite overlay, and dirty-tile flushing.
class Renderer {
public:
  using BackgroundFn = std::function<void(int x0, int y0, int w, int h, int32_t worldX0, int32_t worldY0, uint16_t* buf)>;
  // `worldOffset`/`span` are on the active scroll axis (see HardwareScroller).
  using StripFn      = std::function<void(int32_t worldOffset, int span, uint16_t* buf)>;

  Renderer(FastILI9341& gfx,
           HardwareScroller& scroller,
           SpriteLayer& sprites,
           DirtyRects& dirty,
           int tileW,
           int tileH);

  void setBackgroundRenderer(const BackgroundFn& fn) { bgFn_ = fn; }
  void setStripRenderer(const StripFn& fn) { stripFn_ = fn; }

  // Scrolls the background by `delta` pixels on the active scroll axis and adds
  // dirty rects to clean sprite ghosts caused by the hardware scroll.
  void scroll(int delta, uint16_t* stripBuf, int maxStripLines);
  // Integrates velocity (px/s) and dt (ms) into whole-pixel scroll steps.
  void scrollByVelocity(int speedPxPerSec, uint32_t dtMs, uint16_t* stripBuf, int maxStripLines);
  void resetScrollAccumulator();

  // Marks sprite movement when sprite coordinates are updated manually.
  void markSpriteMovement(const Rect& oldRect, const Rect& newRect);

  // Forces a full-screen redraw on the next flush.
  void invalidate();

  // Flushes dirty rects: background render -> sprite overlay -> blit.
  // `regionBuf` must contain at least tileW*tileH pixels.
  void flush(uint16_t* regionBuf);

private:
  FastILI9341& gfx_;
  HardwareScroller& scroller_;
  SpriteLayer& sprites_;
  DirtyRects& dirty_;
  TileFlusher flusher_;
  BackgroundFn bgFn_{};
  StripFn stripFn_{};
  int tileW_;
  int tileH_;
  int32_t scrollAccumMilliPx_ = 0;  // signed accumulator: px*ms/s remainder in [-999, 999]

  void addSpriteGhosts(int delta);
  static void spriteBounds(const SpriteLayer::Sprite& s, Rect* out);
};
