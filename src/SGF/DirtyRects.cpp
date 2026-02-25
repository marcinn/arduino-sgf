#include "DirtyRects.h"

static inline int16_t i16max(int16_t a, int16_t b){ return a>b?a:b; }
static inline int16_t i16min(int16_t a, int16_t b){ return a<b?a:b; }

bool DirtyRects::overlapsOrTouches(const Rect& a, const Rect& b) {
  // touch-inclusive
  return !(a.x1 < b.x0 - 1 || a.x0 > b.x1 + 1 || a.y1 < b.y0 - 1 || a.y0 > b.y1 + 1);
}

Rect DirtyRects::unite(const Rect& a, const Rect& b) {
  Rect u;
  u.x0 = i16min(a.x0, b.x0);
  u.y0 = i16min(a.y0, b.y0);
  u.x1 = i16max(a.x1, b.x1);
  u.y1 = i16max(a.y1, b.y1);
  return u;
}

bool DirtyRects::add(int x0, int y0, int x1, int y1) {
  if (x1 < x0 || y1 < y0) return false;
  Rect nr{(int16_t)x0,(int16_t)y0,(int16_t)x1,(int16_t)y1};

  // Merge eagerly with existing rects to avoid growth.
  for (int i=0;i<n;i++) {
    if (overlapsOrTouches(r[i], nr)) {
      r[i] = unite(r[i], nr);
      return true;
    }
  }

  if (n >= MAX) {
    // Out of slots: fallback to one large union rect.
    if (n == 0) { r[0] = nr; n = 1; return true; }
    r[0] = unite(r[0], nr);
    for (int i=1;i<n;i++) r[0] = unite(r[0], r[i]);
    n = 1;
    return true;
  }

  r[n++] = nr;
  return true;
}

void DirtyRects::clip(int w, int h) {
  for (int i=0;i<n;i++) {
    if (r[i].x0 < 0) r[i].x0 = 0;
    if (r[i].y0 < 0) r[i].y0 = 0;
    if (r[i].x1 >= w) r[i].x1 = w-1;
    if (r[i].y1 >= h) r[i].y1 = h-1;
    if (r[i].x1 < r[i].x0 || r[i].y1 < r[i].y0) {
      // Remove empty rect.
      r[i] = r[n-1];
      n--;
      i--;
    }
  }
}

void DirtyRects::mergeAll() {
  bool changed = true;
  while (changed) {
    changed = false;
    for (int i=0;i<n && !changed;i++) {
      for (int j=i+1;j<n;j++) {
        if (overlapsOrTouches(r[i], r[j])) {
          r[i] = unite(r[i], r[j]);
          r[j] = r[n-1];
          n--;
          changed = true;
          break;
        }
      }
    }
  }
}
