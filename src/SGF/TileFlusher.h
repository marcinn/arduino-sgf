#pragma once

#include <stdint.h>
#include <functional>

#include "DirtyRects.h"
#include "IRenderTarget.h"

class TileFlusher {
public:
  using RenderRegionFn = std::function<void(int x0, int y0, int w, int h, uint16_t* buf)>;

  TileFlusher(DirtyRects& dirty, int tileW, int tileH)
    : dirty(dirty), tileW(tileW), tileH(tileH) {}

  void flush(IRenderTarget& target, uint16_t* regionBuf, const RenderRegionFn& renderRegion);

private:
  DirtyRects& dirty;
  int tileW;
  int tileH;
};
