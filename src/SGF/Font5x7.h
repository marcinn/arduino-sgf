#pragma once

#include <Arduino.h>

namespace Font5x7 {

using FillRectFn = void (*)(int x, int y, int w, int h, uint16_t color565);
using FillRectCtxFn = void (*)(void* ctx, int x, int y, int w, int h, uint16_t color565);

int textWidth(const char* s, int scale);
bool textPixel(const char* s, int scale, int x, int y);
void drawText(int x, int y, const char* s, int scale, uint16_t color565, FillRectFn fillRect);
void drawText(
  int x,
  int y,
  const char* s,
  int scale,
  uint16_t color565,
  void* ctx,
  FillRectCtxFn fillRect);
void drawCenteredText(int screenW, int y, const char* s, int scale, uint16_t color565, FillRectFn fillRect);
void drawCenteredText(
  int screenW,
  int y,
  const char* s,
  int scale,
  uint16_t color565,
  void* ctx,
  FillRectCtxFn fillRect);

}  // namespace Font5x7
