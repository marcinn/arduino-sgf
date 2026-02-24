#include "RectFlashAnim.h"

RectFlashAnim::RectFlashAnim(RectFlashAnimSlot *slots, int slotCount, uint16_t whiteColor, uint16_t warmWhiteColor)
    : slots_(slots),
      slotCount_(slotCount),
      whiteColor_(whiteColor),
      warmWhiteColor_(warmWhiteColor) {}

void RectFlashAnim::clear() {
  for (int i = 0; i < slotCount_; i++) slots_[i].active = false;
}

void RectFlashAnim::spawn(int x0, int y0, int x1, int y1, uint32_t durationUs, uint16_t baseColor, uint16_t lightColor) {
  int slot = -1;
  for (int i = 0; i < slotCount_; i++) {
    if (!slots_[i].active) {
      slot = i;
      break;
    }
  }
  if (slot < 0) slot = 0;

  RectFlashAnimSlot &f = slots_[slot];
  f.active = true;
  f.remUs = durationUs;
  f.totalUs = durationUs;
  f.x0 = x0;
  f.y0 = y0;
  f.x1 = x1;
  f.y1 = y1;
  f.baseColor = baseColor;
  f.lightColor = lightColor;
}

uint16_t RectFlashAnim::colorAt(int x, int y) const {
  for (int i = 0; i < slotCount_; i++) {
    const RectFlashAnimSlot &f = slots_[i];
    if (!f.active) continue;
    if (x < f.x0 || x > f.x1 || y < f.y0 || y > f.y1) continue;

    if (f.totalUs == 0) return f.baseColor;
    uint64_t rem4 = (uint64_t)f.remUs * 4u;
    uint64_t tot = (uint64_t)f.totalUs;
    if (rem4 > tot * 3u) return whiteColor_;
    if (rem4 > tot * 2u) return warmWhiteColor_;
    if (rem4 > tot) return f.lightColor;
    return f.baseColor;
  }
  return 0;
}

void RectFlashAnim::markDirty(DirtyRects &dirty) const {
  for (int i = 0; i < slotCount_; i++) {
    if (!slots_[i].active) continue;
    dirty.add(slots_[i].x0 - 1, slots_[i].y0 - 1, slots_[i].x1 + 1, slots_[i].y1 + 1);
  }
}

void RectFlashAnim::advance(uint32_t dtUs, DirtyRects &dirty) {
  if (dtUs == 0) return;

  for (int i = 0; i < slotCount_; i++) {
    RectFlashAnimSlot &f = slots_[i];
    if (!f.active) continue;

    if (f.remUs > dtUs) {
      f.remUs -= dtUs;
      continue;
    }

    f.remUs = 0;
    dirty.add(f.x0 - 1, f.y0 - 1, f.x1 + 1, f.y1 + 1);
    f.active = false;
  }
}
