#include "TileFlusher.h"

#include <algorithm>

void TileFlusher::flush(IRenderTarget& target, uint16_t* regionBuf, const RenderRegionFn& renderRegion) {
  if (!renderRegion) return;

  dirty.clip(target.width(), target.height());
  dirty.mergeAll();

  for (int i = 0; i < dirty.count(); i++) {
    const Rect& r = dirty[i];
    for (int y = r.y0; y <= r.y1; y += tileH) {
      int hh = std::min(tileH, r.y1 - y + 1);
      for (int x = r.x0; x <= r.x1; x += tileW) {
        int ww = std::min(tileW, r.x1 - x + 1);
        renderRegion(x, y, ww, hh, regionBuf);
        target.blit565(x, y, ww, hh, regionBuf);
      }
    }
  }
  dirty.clear();
}
