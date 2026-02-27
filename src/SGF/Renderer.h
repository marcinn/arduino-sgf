#pragma once

#include <array>
#include <functional>
#include <stdint.h>

#include "DirtyRects.h"
#include "IRenderTarget.h"
#include "Sprites.h"
#include "TileFlusher.h"
#include "Scroller.h"

// Renderer facade that combines hardware scroll, background redraw,
// sprite overlay, and dirty-tile flushing.
class Renderer {
public:
  using BackgroundFn = std::function<void(int x0, int y0, int w, int h, int32_t worldX0, int32_t worldY0, uint16_t* buf)>;
  struct StripDesc {
    bool alongY = true;   // Active hardware-scroll axis in screen space.
    int span = 0;         // Strip thickness on the active axis.
    int w = 0;            // Buffer width.
    int h = 0;            // Buffer height.
    int32_t worldX0 = 0;  // Background world origin for the strip buffer.
    int32_t worldY0 = 0;
  };
  using StripFn = std::function<void(const StripDesc& strip, uint16_t* buf)>;

  Renderer(IRenderTarget& target,
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
  void configureScroll(uint16_t fixedStart, uint16_t scrollSpan, uint16_t fixedEnd);
  void configureFullScreenScroll();
  void resetScrollOffset(uint16_t offset = 0);

  // Optional manual dirty mark for callers that want explicit control.
  // Renderer also auto-tracks sprite bounds across flushes.
  void markSpriteMovement(const Rect& oldRect, const Rect& newRect);

  // Forces a full-screen redraw on the next flush.
  void invalidate();

  // Flushes dirty rects: background render -> sprite overlay -> blit.
  // `regionBuf` must contain at least tileW*tileH pixels.
  void flush(uint16_t* regionBuf);

private:
  IRenderTarget& target_;
  HardwareScroller scroller_;
  SpriteLayer& sprites_;
  DirtyRects& dirty_;
  TileFlusher flusher_;
  BackgroundFn bgFn_{};
  StripFn stripFn_{};
  int tileW_;
  int tileH_;
  int32_t scrollAccumMilliPx_ = 0;  // signed accumulator: px*ms/s remainder in [-999, 999]
  struct SpriteSnapshot {
    bool active = false;
    Rect bounds{0, 0, 0, 0};
    uint32_t redrawRevision = 0;
  };
  std::array<SpriteSnapshot, SpriteLayer::kMaxSprites> spriteSnapshots_{};

  void addSpriteGhosts(int delta);
  void trackSpriteChanges();
  static void spriteBounds(const SpriteLayer::Sprite& s, Rect* out);
};
