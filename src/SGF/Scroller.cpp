#include "Scroller.h"

#include <algorithm>

void HardwareScroller::configure(uint16_t topFixed, uint16_t scrollHeight, uint16_t bottomFixed) {
  topFixed_ = topFixed;
  scrollH_ = scrollHeight;
  bottomFixed_ = bottomFixed;
  gfx.setScrollArea(topFixed_, scrollH_, bottomFixed_);
  resetOffset(0);
}

void HardwareScroller::resetOffset(uint16_t yOff) {
  if (scrollH_ == 0) return;
  offset_ = (uint16_t)(yOff % scrollH_);
  gfx.scrollTo(offset_);
  // worldTop_ liczymy tak, żeby 0 był początkiem strefy scrolla w czasie startu
  worldTop_ = (int32_t)offset_;
}

void HardwareScroller::scroll(int delta,
                              uint16_t* buf,
                              int maxStripLines,
                              const RenderStripFn& renderStrip,
                              const BlitStripFn& blitStrip) {
  if (!renderStrip || scrollH_ == 0 || maxStripLines <= 0) return;

  // Rozbijamy delta na kroki |step|<=maxStripLines, żeby bufor wystarczył
  int remaining = delta;
  const int w = gfx.width();

  auto stepOnce = [&](int step) {
    // Aktualizacja offsetu (VSCRSADD): dodatni step = obraz w górę, odkryty pasek na dole
    int newOff = (int)offset_ + step;
    while (newOff >= (int)scrollH_) newOff -= (int)scrollH_;
    while (newOff < 0) newOff += (int)scrollH_;
    offset_ = (uint16_t)newOff;
    gfx.scrollTo(offset_);

    // worldTop_: logiczny Y pierwszej linii strefy scrolla
    worldTop_ += step;

    // Odkryty pasek: zależy od kierunku
    int stripH = std::abs(step);
    int yStripPhys;
    if (step > 0) {
      // Scroll w górę: odkryty pasek u dołu
      yStripPhys = offset_ + scrollH_ - stripH;
      if (yStripPhys >= scrollH_) yStripPhys -= scrollH_;
    } else {
      // Scroll w dół: odkryty pasek u góry (offset_ wskazuje nowy top)
      yStripPhys = offset_;
    }

    // Współrzędna w świecie (do generowania terenu/levelu)
    int32_t worldY = (step > 0) ? (worldTop_ + scrollH_ - stripH) : worldTop_;

    renderStrip(worldY, stripH, buf);
    if (blitStrip) {
      blitStrip(yStripPhys, stripH, buf);
    } else {
      gfx.blit565(0, yStripPhys, w, stripH, buf);
    }
  };

  while (remaining != 0) {
    int step = std::clamp(remaining, -maxStripLines, maxStripLines);
    stepOnce(step);
    remaining -= step;
  }
}
