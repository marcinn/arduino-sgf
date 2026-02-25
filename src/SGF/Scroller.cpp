#include "Scroller.h"

#include <algorithm>

void HardwareScroller::configure(uint16_t topFixed, uint16_t scrollHeight, uint16_t bottomFixed) {
  topFixed_ = topFixed;
  scrollH_ = scrollHeight;
  bottomFixed_ = bottomFixed;
  gfx.setScrollArea(topFixed_, scrollH_, bottomFixed_);
  resetOffset(0);
}

void HardwareScroller::configureFullScreen() {
  configure(0, axisLength(), 0);
}

void HardwareScroller::resetOffset(uint16_t yOff) {
  if (scrollH_ == 0) return;
  offset_ = (uint16_t)(yOff % scrollH_);
  gfx.scrollTo((uint16_t)(topFixed_ + offset_));
  // Keep the logical offset aligned with the visible start of the scroll area.
  worldTop_ = (int32_t)offset_;
}

void HardwareScroller::scroll(int delta,
                              uint16_t* buf,
                              int maxStripLines,
                              const RenderStripFn& renderStrip,
                              const BlitStripFn& blitStrip) {
  if (!renderStrip || scrollH_ == 0 || maxStripLines <= 0) return;

  // Split large deltas into chunks so the strip buffer always fits.
  int remaining = delta;
  const bool alongY = scrollsAlongY();
  const bool inverted = axisInverted();
  const int cross = alongY ? gfx.width() : gfx.height();

  auto stepOnce = [&](int step) {
    // Update VSCRSADD. Positive step advances the logical scroll offset.
    int newOff = (int)offset_ + step;
    while (newOff >= (int)scrollH_) newOff -= (int)scrollH_;
    while (newOff < 0) newOff += (int)scrollH_;
    offset_ = (uint16_t)newOff;
    gfx.scrollTo((uint16_t)(topFixed_ + offset_));

    // Logical offset of the first visible unit in the scroll area.
    worldTop_ += step;

    // Newly exposed strip position in logical screen coordinates on the active axis.
    const int stripSpan = std::abs(step);
    const int screenStripStart = (step > 0) ? ((int)scrollH_ - stripSpan) : 0;
    int stripPhys = inverted ? (screenStripStart - (int)offset_) : ((int)offset_ + screenStripStart);
    while (stripPhys >= (int)scrollH_) stripPhys -= (int)scrollH_;
    while (stripPhys < 0) stripPhys += (int)scrollH_;

    // Logical coordinate of the exposed strip (for world generation).
    const int32_t worldOffset = (step > 0) ? (worldTop_ + scrollH_ - stripSpan) : worldTop_;
    const int physPosOnScreen = (int)topFixed_ + stripPhys;

    renderStrip(worldOffset, stripSpan, buf);
    if (blitStrip) {
      blitStrip(physPosOnScreen, stripSpan, buf);
    } else if (alongY) {
      gfx.blit565(0, physPosOnScreen, cross, stripSpan, buf);
    } else {
      gfx.blit565(physPosOnScreen, 0, stripSpan, cross, buf);
    }
  };

  while (remaining != 0) {
    int step = std::clamp(remaining, -maxStripLines, maxStripLines);
    stepOnce(step);
    remaining -= step;
  }
}
