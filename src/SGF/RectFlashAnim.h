#pragma once

#include <Arduino.h>
#include "DirtyRects.h"

struct RectFlashAnimSlot {
  bool active;
  uint32_t remUs;
  uint32_t totalUs;
  int x0, y0, x1, y1;
  uint16_t baseColor;
  uint16_t lightColor;
};

class RectFlashAnim {
public:
  RectFlashAnim(RectFlashAnimSlot *slots, int slotCount, uint16_t whiteColor, uint16_t warmWhiteColor);

  void clear();
  void spawn(int x0, int y0, int x1, int y1, uint32_t durationUs, uint16_t baseColor, uint16_t lightColor);
  uint16_t colorAt(int x, int y) const;
  void markDirty(DirtyRects &dirty) const;
  void advance(uint32_t dtUs, DirtyRects &dirty);

private:
  RectFlashAnimSlot *slots_;
  int slotCount_;
  uint16_t whiteColor_;
  uint16_t warmWhiteColor_;
};
