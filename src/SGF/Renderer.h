#pragma once

#include <functional>
#include <stdint.h>

#include "DirtyRects.h"
#include "Sprites.h"
#include "TileFlusher.h"
#include "Scroller.h"

// Renderer: fasada spinająca hardware scroll (ILI9341), tło i sprite'y.
// Zakłada jedno źródło tła (callback) i jeden SpriteLayer.
class Renderer {
public:
  using BackgroundFn = std::function<void(int x0, int y0, int w, int h, int32_t worldX0, int32_t worldY0, uint16_t* buf)>;
  using StripFn      = std::function<void(int32_t worldY, int h, uint16_t* buf)>;

  Renderer(FastILI9341& gfx,
           HardwareScroller& scroller,
           SpriteLayer& sprites,
           DirtyRects& dirty,
           int tileW,
           int tileH);

  void setBackgroundRenderer(const BackgroundFn& fn) { bgFn_ = fn; }
  void setStripRenderer(const StripFn& fn) { stripFn_ = fn; }

  // Scrolluje tło o delta pikseli; dodaje dirty dla sprite'ów (ghost cleanup).
  void scroll(int delta, uint16_t* stripBuf, int maxStripLines);
  // Integruje prędkość (px/s) i dt (ms) do scrolla o całe piksele.
  void scrollByVelocity(int speedPxPerSec, uint32_t dtMs, uint16_t* stripBuf, int maxStripLines);
  void resetScrollAccumulator();

  // Pomocniczo: zaznacz ruch sprite'a gdy przesuwasz go manualnie (stary i nowy rect).
  void markSpriteMovement(const Rect& oldRect, const Rect& newRect);

  // Wymusza pełne odświeżenie.
  void invalidate();

  // Flushuje dirty recty: render tła -> sprite'y -> blit.
  // regionBuf musi mieć tileW*tileH elementów.
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
  int32_t scrollAccumMilliPx_ = 0;  // signed accumulator: px*ms/s (remainder in [-999,999])

  void addSpriteGhosts(int delta);
  static void spriteBounds(const SpriteLayer::Sprite& s, Rect* out);
};
