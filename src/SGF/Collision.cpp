#include "Collision.h"

bool circleRectHit(int cx, int cy, int r, int x0, int y0, int x1, int y1) {
  int nx = cx;
  if (nx < x0) nx = x0;
  else if (nx > x1) nx = x1;

  int ny = cy;
  if (ny < y0) ny = y0;
  else if (ny > y1) ny = y1;

  int dx = cx - nx;
  int dy = cy - ny;
  return dx * dx + dy * dy <= r * r;
}

bool aabbHit(int ax0, int ay0, int ax1, int ay1, int bx0, int by0, int bx1, int by1) {
  if (ax1 < bx0 || bx1 < ax0) return false;
  if (ay1 < by0 || by1 < ay0) return false;
  return true;
}

bool circleCircleHit(int ax, int ay, int ar, int bx, int by, int br) {
  int dx = ax - bx;
  int dy = ay - by;
  int rsum = ar + br;
  return dx * dx + dy * dy <= rsum * rsum;
}

bool pointInRect(int px, int py, int x0, int y0, int x1, int y1) {
  return px >= x0 && px <= x1 && py >= y0 && py <= y1;
}

bool raycastToRect(int ox, int oy, int dx, int dy, int x0, int y0, int x1, int y1, float* tHit) {
  // Liang-Barsky / slab method for axis-aligned rect. Returns first hit t in [0,1] if provided.
  float t0 = 0.0f, t1 = 1.0f;

  auto update = [&](int p, int q) -> bool {
    if (p == 0) return q >= 0;  // Parallel: inside slab if q>=0
    float t = (float)q / (float)p;
    if (p < 0) {
      if (t > t1) return false;
      if (t > t0) t0 = t;
    } else {
      if (t < t0) return false;
      if (t < t1) t1 = t;
    }
    return true;
  };

  if (!update(-dx, ox - x0)) return false;
  if (!update( dx, x1 - ox)) return false;
  if (!update(-dy, oy - y0)) return false;
  if (!update( dy, y1 - oy)) return false;

  if (tHit) *tHit = t0;
  return true;
}
