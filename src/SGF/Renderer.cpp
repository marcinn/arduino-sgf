#include "Renderer.h"

#include <algorithm>

namespace {

// Maps logical screen Y to physical GRAM rows while hardware vertical scroll is active.
// TileFlusher works in screen coordinates, so Renderer::flush uses this adapter to blit
// dirty tiles into the correct RAM rows (including wrap inside the scroll region).
class ScrolledRenderTarget : public IRenderTarget {
public:
  ScrolledRenderTarget(FastILI9341& gfx, const HardwareScroller& scroller)
    : gfx_(gfx), scroller_(scroller) {}

  int width() const override { return gfx_.width(); }
  int height() const override { return gfx_.height(); }

  void blit565(int x0, int y0, int w, int h, const uint16_t* pix) override {
    if (!pix || w <= 0 || h <= 0) return;

    const int top = (int)scroller_.topFixed();
    const int scrollH = (int)scroller_.scrollHeight();
    if (scrollH <= 0) {
      gfx_.blit565(x0, y0, w, h, pix);
      return;
    }

    const int bottom = top + scrollH;
    const int offset = (int)scroller_.offset();

    int row = 0;
    while (row < h) {
      const int sy = y0 + row;

      // Fixed areas are not remapped by hardware scroll.
      if (sy < top) {
        const int segH = std::min(h - row, top - sy);
        gfx_.blit565(x0, sy, w, segH, pix + row * w);
        row += segH;
        continue;
      }
      if (sy >= bottom) {
        gfx_.blit565(x0, sy, w, h - row, pix + row * w);
        return;
      }

      // Scroll area: split if the physical address wraps inside this tile.
      const int localY = sy - top;
      const int physLocalY = (offset + localY) % scrollH;
      const int physY = top + physLocalY;
      const int bySource = std::min(h - row, bottom - sy);
      const int byWrap = scrollH - physLocalY;
      const int segH = std::min(bySource, byWrap);

      gfx_.blit565(x0, physY, w, segH, pix + row * w);
      row += segH;
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

  // Rysujemy pasek odkryty przez hardware scroll.
  scroller_.scroll(
    delta,
    stripBuf,
    maxStripLines,
    [&](int32_t worldY, int h, uint16_t* buf) {
      if (stripFn_) {
        stripFn_(worldY, h, buf);
      } else if (bgFn_) {
        // Render tła w pasku (pełna szerokość, worldY dostarczony).
        bgFn_(0, 0, gfx_.width(), h, 0, worldY, buf);
      } else {
        // brak callbacku — wyczyść na czarno
        std::fill(buf, buf + gfx_.width() * h, (uint16_t)0);
      }
    },
    [&](int physY, int h, uint16_t* buf) {
      gfx_.blit565(0, physY, gfx_.width(), h, buf);
    });

  // Dodajemy dirty na sprite'y, żeby nadpisać „duchy”.
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
  for (int i = 0; i < SpriteLayer::kMaxSprites; ++i) {
    const auto& s = sprites_.sprite(i);
    if (!s.active) continue;
    Rect r;
    spriteBounds(s, &r);
    // Aktualna pozycja
    dirty_.add(r.x0, r.y0, r.x1, r.y1);
    // Ghost po hardware scrollu: ekran przesunął się o delta
    Rect g = r;
    g.y0 -= delta;
    g.y1 -= delta;
    dirty_.add(g.x0, g.y0, g.x1, g.y1);
  }
}

void Renderer::markSpriteMovement(const Rect& oldRect, const Rect& newRect) {
  dirty_.add(oldRect.x0, oldRect.y0, oldRect.x1, oldRect.y1);
  dirty_.add(newRect.x0, newRect.y0, newRect.x1, newRect.y1);
}

void Renderer::invalidate() {
  dirty_.invalidate(gfx_);
}

static int32_t worldYFromScreenY(const HardwareScroller& scroller, int y) {
  uint16_t top = scroller.topFixed();
  uint16_t scrollH = scroller.scrollHeight();
  if (y < (int)top || scrollH == 0) {
    return y;  // HUD / nieruchoma część
  }
  if (y >= (int)(top + scrollH)) {
    return y;  // bottom fixed area
  }
  return scroller.worldTop() + (y - (int)top);
}

void Renderer::flush(uint16_t* regionBuf) {
  if (!regionBuf) return;

  ScrolledRenderTarget target(gfx_, scroller_);
  flusher_.flush(
    target,
    regionBuf,
    [&](int x0, int y0, int w, int h, uint16_t* buf) {
      int32_t worldY = worldYFromScreenY(scroller_, y0);
      if (bgFn_) {
        bgFn_(x0, y0, w, h, x0, worldY, buf);
      } else {
        std::fill(buf, buf + w * h, (uint16_t)0);
      }
      sprites_.renderRegion(x0, y0, w, h, buf);
    });
}

void Renderer::spriteBounds(const SpriteLayer::Sprite& s, Rect* out) {
  SpriteLayer::spriteBounds(s, &out->x0, &out->y0, &out->x1, &out->y1);
}
