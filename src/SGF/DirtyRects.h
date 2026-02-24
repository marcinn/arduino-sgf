#pragma once
#include <Arduino.h>

struct Rect {
  int16_t x0, y0, x1, y1; // inclusive
};

class DirtyRects {
public:
  static constexpr int MAX = 32;

  void clear() { n = 0; }

  bool add(int x0, int y0, int x1, int y1);
  void clip(int w, int h);

  int count() const { return n; }
  const Rect& operator[](int i) const { return r[i]; }

  // Prosty merge: jeśli recty się przecinają lub stykają, łączy.
  void mergeAll();

private:
  Rect r[MAX];
  int n = 0;

  static bool overlapsOrTouches(const Rect& a, const Rect& b);
  static Rect unite(const Rect& a, const Rect& b);
};
