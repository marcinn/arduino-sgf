#pragma once

#include <stdbool.h>

bool circleRectHit(int cx, int cy, int r, int x0, int y0, int x1, int y1);
bool aabbHit(int ax0, int ay0, int ax1, int ay1, int bx0, int by0, int bx1, int by1);
bool circleCircleHit(int ax, int ay, int ar, int bx, int by, int br);
bool pointInRect(int px, int py, int x0, int y0, int x1, int y1);
bool raycastToRect(int ox, int oy, int dx, int dy, int x0, int y0, int x1, int y1, float* tHit);
