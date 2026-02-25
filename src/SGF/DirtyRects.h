#pragma once
#include <Arduino.h>
#include "IRenderTarget.h"

struct Rect {
  int16_t x0, y0, x1, y1; // inclusive
};

class DirtyRects {
public:
  static constexpr int MAX = 32;

  void clear() { n = 0; }
  void invalidate(const IRenderTarget& target) {
    clear();
    if (target.width() > 0 && target.height() > 0) {
      add(0, 0, target.width() - 1, target.height() - 1);
    }
  }

  bool add(int x0, int y0, int x1, int y1);
  void clip(int w, int h);

  int count() const { return n; }
  const Rect& operator[](int i) const { return r[i]; }

  // Simple merge: joins rects that overlap or touch.
  void mergeAll();

private:
  Rect r[MAX];
  int n = 0;

  static bool overlapsOrTouches(const Rect& a, const Rect& b);
  static Rect unite(const Rect& a, const Rect& b);
};
