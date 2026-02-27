#include "Scroller.h"

#include <algorithm>

namespace {

// Blits a sub-rectangle of a row-major strip buffer by sending one display row
// at a time. Used for landscape strip wrap, where the source split is by X and
// the sub-rect columns are not contiguous in memory.
static void blitStripSubRectRows(IRenderTarget& target,
                                 int dstX,
                                 int dstY,
                                 int w,
                                 int h,
                                 const uint16_t* src,
                                 int srcStride,
                                 int srcX) {
  if (!src || w <= 0 || h <= 0) return;
  for (int row = 0; row < h; ++row) {
    target.blit565(dstX, dstY + row, w, 1, src + row * srcStride + srcX);
  }
}

}  // namespace

void HardwareScroller::configure(uint16_t topFixed, uint16_t scrollHeight, uint16_t bottomFixed) {
  topFixed_ = topFixed;
  scrollH_ = scrollHeight;
  bottomFixed_ = bottomFixed;
  if (hardwareEnabled_) {
    target_.setScrollArea(topFixed_, scrollH_, bottomFixed_);
  }
  resetOffset(0);
}

void HardwareScroller::configureFullScreen() {
  configure(0, axisLength(), 0);
}

void HardwareScroller::resetOffset(uint16_t yOff) {
  if (scrollH_ == 0) return;
  offset_ = (uint16_t)(yOff % scrollH_);
  if (hardwareEnabled_) {
    target_.scrollTo((uint16_t)(topFixed_ + offset_));
  }
  // Keep the logical offset aligned with the visible start of the scroll area.
  worldTop_ = (int32_t)offset_;
}

void HardwareScroller::scroll(int delta,
                              uint16_t* buf,
                              int maxStripLines,
                              const RenderStripFn& renderStrip,
                              const BlitStripFn& blitStrip) {
  if (scrollH_ == 0 || maxStripLines <= 0) return;

  if (!hardwareEnabled_) {
    worldTop_ += delta;
    return;
  }

  if (!renderStrip) return;

  // Split large deltas into chunks so the strip buffer always fits.
  int remaining = delta;
  const bool alongY = scrollsAlongY();
  const int cross = alongY ? target_.width() : target_.height();

  const bool inverted = axisInverted();

  auto stepOnce = [&](int step) {
    // Update the raw VSCRSADD value. In mirrored-axis rotations the hardware
    // scroll address must move in the opposite direction to keep API `delta`
    // semantics stable in screen space.
    const int hwStep = inverted ? -step : step;
    int newOff = (int)offset_ + hwStep;
    while (newOff >= (int)scrollH_) newOff -= (int)scrollH_;
    while (newOff < 0) newOff += (int)scrollH_;
    offset_ = (uint16_t)newOff;
    target_.scrollTo((uint16_t)(topFixed_ + offset_));

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
      const int spanEnd = (int)topFixed_ + (int)scrollH_;
      const int firstSpan = std::min(stripSpan, spanEnd - physPosOnScreen);
      target_.blit565(0, physPosOnScreen, cross, firstSpan, buf);
      const int secondSpan = stripSpan - firstSpan;
      if (secondSpan > 0) {
        target_.blit565(0, (int)topFixed_, cross, secondSpan, buf + cross * firstSpan);
      }
    } else {
      const int spanEnd = (int)topFixed_ + (int)scrollH_;
      const int firstSpan = std::min(stripSpan, spanEnd - physPosOnScreen);
      const int secondSpan = stripSpan - firstSpan;
      if (secondSpan <= 0) {
        target_.blit565(physPosOnScreen, 0, stripSpan, cross, buf);
      } else {
        blitStripSubRectRows(target_, physPosOnScreen, 0, firstSpan, cross, buf, stripSpan, 0);
        blitStripSubRectRows(
          target_,
          (int)topFixed_,
          0,
          secondSpan,
          cross,
          buf,
          stripSpan,
          firstSpan
        );
      }
    }
  };

  while (remaining != 0) {
    const int stepLimit = std::min<int>(maxStripLines, scrollH_);
    int step = std::clamp(remaining, -stepLimit, stepLimit);
    stepOnce(step);
    remaining -= step;
  }
}
