#include "Renderer.h"

#include <algorithm>

namespace {

static void blitSubRectRows(IRenderTarget& target,
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
    target.blit565(dstX, dstY, w, h, src);
    return;
  }

  if (h == 1) {
    target.blit565(dstX, dstY, w, 1, src + srcX);
    return;
  }

  for (int row = 0; row < h; ++row) {
    target.blit565(dstX, dstY + row, w, 1, src + row * srcStride + srcX);
  }
}

// Maps logical screen coordinates to physical GRAM positions while hardware
// scroll is active. TileFlusher works in screen coordinates, so Renderer::flush
// uses this adapter to write dirty tiles to the correct wrapped position.
class ScrolledRenderTarget : public IRenderTarget {
public:
  ScrolledRenderTarget(IRenderTarget& target, const HardwareScroller& scroller)
    : target(target), scroller(scroller) {}

  int width() const override { return target.width(); }
  int height() const override { return target.height(); }

  void blit565(int x0, int y0, int w, int h, const uint16_t* pix) override {
    if (!pix || w <= 0 || h <= 0) return;

    const int fixedStart = (int)scroller.fixedStart();
    const int scrollSpan = (int)scroller.scrollSpan();
    if (!scroller.usesHardwareScroll() || scrollSpan <= 0) {
      target.blit565(x0, y0, w, h, pix);
      return;
    }

    const int fixedEndStart = fixedStart + scrollSpan;
    const int offset = (int)scroller.offset();
    const bool alongY = scroller.scrollsAlongY();
    const bool inverted = scroller.axisInverted();

    if (alongY) {
      int row = 0;
      while (row < h) {
        const int sy = y0 + row;

        // Fixed areas are not remapped by hardware scroll.
        if (sy < fixedStart) {
          const int segH = std::min(h - row, fixedStart - sy);
          target.blit565(x0, sy, w, segH, pix + row * w);
          row += segH;
          continue;
        }
        if (sy >= fixedEndStart) {
          target.blit565(x0, sy, w, h - row, pix + row * w);
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

        target.blit565(x0, physY, w, segH, pix + row * w);
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
        blitSubRectRows(target, sx, y0, segW, h, pix, w, col);
        col += segW;
        continue;
      }
      if (sx >= fixedEndStart) {
        blitSubRectRows(target, sx, y0, w - col, h, pix, w, col);
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

      blitSubRectRows(target, physX, y0, segW, h, pix, w, col);
      col += segW;
    }
  }

private:
  IRenderTarget& target;
  const HardwareScroller& scroller;
};

}  // namespace

Renderer::Renderer(IRenderTarget& target,
                   SpriteLayer& sprites,
                   DirtyRects& dirty,
                   int tileW,
                   int tileH)
  : target(target),
    scroller(target),
    sprites(sprites),
    dirty(dirty),
    flusher(dirty, tileW, tileH),
    tileW(tileW),
    tileH(tileH) {}

void Renderer::configureScroll(uint16_t fixedStart, uint16_t scrollSpan, uint16_t fixedEnd) {
  scroller.configure(fixedStart, scrollSpan, fixedEnd);
}

void Renderer::configureFullScreenScroll() {
  scroller.configureFullScreen();
}

void Renderer::resetScrollOffset(uint16_t offset) {
  scroller.resetOffset(offset);
}

void Renderer::scroll(int delta, uint16_t* stripBuf, int maxStripLines) {
  if (delta == 0) return;

  if (!scroller.usesHardwareScroll()) {
    scroller.scroll(delta, stripBuf, maxStripLines, {});
    dirty.invalidate(target);
    return;
  }

  // Render the strip exposed by the hardware scroll.
  scroller.scroll(
    delta,
    stripBuf,
    maxStripLines,
    [&](int32_t worldOffset, int span, uint16_t* buf) {
      Renderer::StripDesc strip{};
      strip.alongY = scroller.scrollsAlongY();
      strip.span = span;
      if (strip.alongY) {
        strip.w = target.width();
        strip.h = span;
        strip.worldX0 = 0;
        strip.worldY0 = worldOffset;
      } else {
        strip.w = span;
        strip.h = target.height();
        strip.worldX0 = worldOffset;
        strip.worldY0 = 0;
      }

      if (stripFn) {
        stripFn(strip, buf);
      } else if (bgFn) {
        // Fallback strip render via the main background callback.
        bgFn(0, 0, strip.w, strip.h, strip.worldX0, strip.worldY0, buf);
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

  scrollAccumMilliPx += (int32_t)speedPxPerSec * (int32_t)dtMs;
  int delta = (int)(scrollAccumMilliPx / 1000);
  scrollAccumMilliPx -= (int32_t)delta * 1000;
  if (delta != 0) {
    scroll(delta, stripBuf, maxStripLines);
  }
}

void Renderer::resetScrollAccumulator() {
  scrollAccumMilliPx = 0;
}

void Renderer::addSpriteGhosts(int delta) {
  if (delta == 0) return;
  const bool alongY = scroller.scrollsAlongY();
  const int screenShift = -delta;
  constexpr int ghostPad = 1;  // Covers edge pixels at tile/sprite boundaries.
  for (int i = 0; i < SpriteLayer::kMaxSprites; ++i) {
    const auto& s = sprites.sprite(i);
    if (!s.active) continue;
    Rect r;
    spriteBounds(s, &r);
    r.x0 -= ghostPad;
    r.y0 -= ghostPad;
    r.x1 += ghostPad;
    r.y1 += ghostPad;
    // Current on-screen position.
    dirty.add(r.x0, r.y0, r.x1, r.y1);
    // Ghost left behind after the screen content shifts by `delta`.
    Rect g = r;
    if (alongY) {
      g.y0 += screenShift;
      g.y1 += screenShift;
    } else {
      g.x0 += screenShift;
      g.x1 += screenShift;
    }
    dirty.add(g.x0, g.y0, g.x1, g.y1);
  }
}

void Renderer::trackSpriteChanges() {
  for (int i = 0; i < SpriteLayer::kMaxSprites; ++i) {
    const auto& s = sprites.sprite(i);
    Rect cur{};
    const bool curActive = s.active;
    const uint32_t curRedrawRevision = s.redrawRevision();
    if (curActive) {
      spriteBounds(s, &cur);
    }

    auto& snap = spriteSnapshots[i];
    bool changed = (snap.active != curActive);
    if (!changed && curActive) {
      changed = (snap.bounds.x0 != cur.x0) || (snap.bounds.y0 != cur.y0) ||
                (snap.bounds.x1 != cur.x1) || (snap.bounds.y1 != cur.y1);
    }
    if (!changed && curActive) {
      changed = (snap.redrawRevision != curRedrawRevision);
    }

    if (changed) {
      if (snap.active) {
        dirty.add(snap.bounds.x0, snap.bounds.y0, snap.bounds.x1, snap.bounds.y1);
      }
      if (curActive) {
        dirty.add(cur.x0, cur.y0, cur.x1, cur.y1);
      }
    }

    snap.active = curActive;
    snap.redrawRevision = curRedrawRevision;
    if (curActive) {
      snap.bounds = cur;
    }
  }
}

void Renderer::markSpriteMovement(const Rect& oldRect, const Rect& newRect) {
  dirty.add(oldRect.x0, oldRect.y0, oldRect.x1, oldRect.y1);
  dirty.add(newRect.x0, newRect.y0, newRect.x1, newRect.y1);
}

void Renderer::invalidate() {
  dirty.invalidate(target);
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
  target.tickEffects();
  if (!regionBuf) return;

  trackSpriteChanges();

  ScrolledRenderTarget bgTarget(target, scroller);
  flusher.flush(
    bgTarget,
    regionBuf,
    [&](int x0, int y0, int w, int h, uint16_t* buf) {
      int32_t worldX = x0;
      int32_t worldY = y0;
      if (scroller.scrollsAlongY()) {
        worldY = worldOffsetFromScreenCoord(scroller, y0);
      } else {
        worldX = worldOffsetFromScreenCoord(scroller, x0);
      }
      if (bgFn) {
        bgFn(x0, y0, w, h, worldX, worldY, buf);
      } else {
        std::fill(buf, buf + w * h, (uint16_t)0);
      }
      sprites.renderRegion(x0, y0, w, h, buf);
    });
}

void Renderer::spriteBounds(const SpriteLayer::Sprite& s, Rect* out) {
  SpriteLayer::spriteBounds(s, &out->x0, &out->y0, &out->x1, &out->y1);
}
