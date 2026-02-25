#include "Renderer.h"

#include <algorithm>

namespace {

static void blitSubRectRows(FastILI9341& gfx,
                            int dstX,
                            int dstY,
                            int w,
                            int h,
                            const uint16_t* src,
                            int srcStride,
                            int srcX) {
  if (w <= 0 || h <= 0 || !src) return;

  // Use a single transfer when the requested sub-rect is contiguous.
  if (srcX == 0 && srcStride == w) {
    gfx.blit565(dstX, dstY, w, h, src);
    return;
  }

  if (h == 1) {
    gfx.blit565(dstX, dstY, w, 1, src + srcX);
    return;
  }

  for (int row = 0; row < h; ++row) {
    gfx.blit565(dstX, dstY + row, w, 1, src + row * srcStride + srcX);
  }
}

// Maps logical screen coordinates to physical GRAM positions while hardware
// scroll is active. TileFlusher works in screen coordinates, so Renderer::flush
// uses this adapter to write dirty tiles to the correct wrapped position.
class ScrolledRenderTarget : public IRenderTarget {
public:
  ScrolledRenderTarget(FastILI9341& gfx, const HardwareScroller& scroller)
    : gfx_(gfx), scroller_(scroller) {}

  int width() const override { return gfx_.width(); }
  int height() const override { return gfx_.height(); }

  void blit565(int x0, int y0, int w, int h, const uint16_t* pix) override {
    if (!pix || w <= 0 || h <= 0) return;

    const int fixedStart = (int)scroller_.fixedStart();
    const int scrollSpan = (int)scroller_.scrollSpan();
    if (scrollSpan <= 0) {
      gfx_.blit565(x0, y0, w, h, pix);
      return;
    }

    const int fixedEndStart = fixedStart + scrollSpan;
    const int offset = (int)scroller_.offset();
    const bool alongY = scroller_.scrollsAlongY();
    const bool inverted = scroller_.axisInverted();

    if (alongY) {
      int row = 0;
      while (row < h) {
        const int sy = y0 + row;

        // Fixed areas are not remapped by hardware scroll.
        if (sy < fixedStart) {
          const int segH = std::min(h - row, fixedStart - sy);
          gfx_.blit565(x0, sy, w, segH, pix + row * w);
          row += segH;
          continue;
        }
        if (sy >= fixedEndStart) {
          gfx_.blit565(x0, sy, w, h - row, pix + row * w);
          return;
        }

        // Scroll area: split if the physical address wraps inside this tile.
        const int localY = sy - fixedStart;
        int physLocalY = inverted ? (localY - offset) : (offset + localY);
        while (physLocalY >= scrollSpan) physLocalY -= scrollSpan;
        while (physLocalY < 0) physLocalY += scrollSpan;
        const int physY = fixedStart + physLocalY;
        const int bySource = std::min(h - row, fixedEndStart - sy);
        const int byWrap = scrollSpan - physLocalY;
        const int segH = std::min(bySource, byWrap);

        gfx_.blit565(x0, physY, w, segH, pix + row * w);
        row += segH;
      }
      return;
    }

    // Landscape path: the hardware scroll axis maps to screen X, so wrapping is
    // horizontal. We split by columns and send row-sized chunks.
    int col = 0;
    while (col < w) {
      const int sx = x0 + col;

      if (sx < fixedStart) {
        const int segW = std::min(w - col, fixedStart - sx);
        blitSubRectRows(gfx_, sx, y0, segW, h, pix, w, col);
        col += segW;
        continue;
      }
      if (sx >= fixedEndStart) {
        blitSubRectRows(gfx_, sx, y0, w - col, h, pix, w, col);
        return;
      }

      const int localX = sx - fixedStart;
      int physLocalX = inverted ? (localX - offset) : (offset + localX);
      while (physLocalX >= scrollSpan) physLocalX -= scrollSpan;
      while (physLocalX < 0) physLocalX += scrollSpan;
      const int physX = fixedStart + physLocalX;
      const int bySource = std::min(w - col, fixedEndStart - sx);
      const int byWrap = scrollSpan - physLocalX;
      const int segW = std::min(bySource, byWrap);

      blitSubRectRows(gfx_, physX, y0, segW, h, pix, w, col);
      col += segW;
    }
  }

private:
  FastILI9341& gfx_;
  const HardwareScroller& scroller_;
};

}  // namespace

Renderer::Renderer(FastILI9341& gfx,
                   HardwareScroller& scroller,
                   SpriteLayer& sprites,
                   DirtyRects& dirty,
                   int tileW,
                   int tileH)
  : gfx_(gfx),
    scroller_(scroller),
    sprites_(sprites),
    dirty_(dirty),
    flusher_(dirty, tileW, tileH),
    tileW_(tileW),
    tileH_(tileH) {}

void Renderer::scroll(int delta, uint16_t* stripBuf, int maxStripLines) {
  if (delta == 0) return;

  // Render the strip exposed by the hardware scroll.
  scroller_.scroll(
    delta,
    stripBuf,
    maxStripLines,
    [&](int32_t worldOffset, int span, uint16_t* buf) {
      Renderer::StripDesc strip{};
      strip.alongY = scroller_.scrollsAlongY();
      strip.span = span;
      if (strip.alongY) {
        strip.w = gfx_.width();
        strip.h = span;
        strip.worldX0 = 0;
        strip.worldY0 = worldOffset;
      } else {
        strip.w = span;
        strip.h = gfx_.height();
        strip.worldX0 = worldOffset;
        strip.worldY0 = 0;
      }

      if (stripFn_) {
        stripFn_(strip, buf);
      } else if (bgFn_) {
        // Fallback strip render via the main background callback.
        bgFn_(0, 0, strip.w, strip.h, strip.worldX0, strip.worldY0, buf);
      } else {
        const int count = strip.w * strip.h;
        std::fill(buf, buf + count, (uint16_t)0);
      }
    });

  // Mark sprite regions so ghosts left by hardware scroll get redrawn.
  addSpriteGhosts(delta);
}

void Renderer::scrollByVelocity(int speedPxPerSec,
                                uint32_t dtMs,
                                uint16_t* stripBuf,
                                int maxStripLines) {
  if (dtMs == 0 || speedPxPerSec == 0) return;

  scrollAccumMilliPx_ += (int32_t)speedPxPerSec * (int32_t)dtMs;
  int delta = (int)(scrollAccumMilliPx_ / 1000);
  scrollAccumMilliPx_ -= (int32_t)delta * 1000;
  if (delta != 0) {
    scroll(delta, stripBuf, maxStripLines);
  }
}

void Renderer::resetScrollAccumulator() {
  scrollAccumMilliPx_ = 0;
}

void Renderer::addSpriteGhosts(int delta) {
  if (delta == 0) return;
  const bool alongY = scroller_.scrollsAlongY();
  const int screenShift = -delta;
  constexpr int ghostPad = 1;  // Covers edge pixels at tile/sprite boundaries.
  for (int i = 0; i < SpriteLayer::kMaxSprites; ++i) {
    const auto& s = sprites_.sprite(i);
    if (!s.active) continue;
    Rect r;
    spriteBounds(s, &r);
    r.x0 -= ghostPad;
    r.y0 -= ghostPad;
    r.x1 += ghostPad;
    r.y1 += ghostPad;
    // Current on-screen position.
    dirty_.add(r.x0, r.y0, r.x1, r.y1);
    // Ghost left behind after the screen content shifts by `delta`.
    Rect g = r;
    if (alongY) {
      g.y0 += screenShift;
      g.y1 += screenShift;
    } else {
      g.x0 += screenShift;
      g.x1 += screenShift;
    }
    dirty_.add(g.x0, g.y0, g.x1, g.y1);
  }
}

void Renderer::trackSpriteChanges() {
  for (int i = 0; i < SpriteLayer::kMaxSprites; ++i) {
    const auto& s = sprites_.sprite(i);
    Rect cur{};
    const bool curActive = s.active;
    if (curActive) {
      spriteBounds(s, &cur);
    }

    auto& snap = spriteSnapshots_[i];
    bool changed = (snap.active != curActive);
    if (!changed && curActive) {
      changed = (snap.bounds.x0 != cur.x0) || (snap.bounds.y0 != cur.y0) ||
                (snap.bounds.x1 != cur.x1) || (snap.bounds.y1 != cur.y1);
    }

    if (changed) {
      if (snap.active) {
        dirty_.add(snap.bounds.x0, snap.bounds.y0, snap.bounds.x1, snap.bounds.y1);
      }
      if (curActive) {
        dirty_.add(cur.x0, cur.y0, cur.x1, cur.y1);
      }
    }

    snap.active = curActive;
    if (curActive) {
      snap.bounds = cur;
    }
  }
}

void Renderer::markSpriteMovement(const Rect& oldRect, const Rect& newRect) {
  dirty_.add(oldRect.x0, oldRect.y0, oldRect.x1, oldRect.y1);
  dirty_.add(newRect.x0, newRect.y0, newRect.x1, newRect.y1);
}

void Renderer::invalidate() {
  dirty_.invalidate(gfx_);
}

static int32_t worldOffsetFromScreenCoord(const HardwareScroller& scroller, int pos) {
  const uint16_t start = scroller.fixedStart();
  const uint16_t span = scroller.scrollSpan();
  if (pos < (int)start || span == 0) {
    return pos;  // Fixed region before the scroll span.
  }
  if (pos >= (int)(start + span)) {
    return pos;  // Fixed region after the scroll span.
  }
  return scroller.worldOffset() + (pos - (int)start);
}

void Renderer::flush(uint16_t* regionBuf) {
  if (!regionBuf) return;

  trackSpriteChanges();

  ScrolledRenderTarget target(gfx_, scroller_);
  flusher_.flush(
    target,
    regionBuf,
    [&](int x0, int y0, int w, int h, uint16_t* buf) {
      int32_t worldX = x0;
      int32_t worldY = y0;
      if (scroller_.scrollsAlongY()) {
        worldY = worldOffsetFromScreenCoord(scroller_, y0);
      } else {
        worldX = worldOffsetFromScreenCoord(scroller_, x0);
      }
      if (bgFn_) {
        bgFn_(x0, y0, w, h, worldX, worldY, buf);
      } else {
        std::fill(buf, buf + w * h, (uint16_t)0);
      }
      sprites_.renderRegion(x0, y0, w, h, buf);
    });
}

void Renderer::spriteBounds(const SpriteLayer::Sprite& s, Rect* out) {
  SpriteLayer::spriteBounds(s, &out->x0, &out->y0, &out->x1, &out->y1);
}
