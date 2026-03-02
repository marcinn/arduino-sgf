#pragma once

#include <Arduino.h>

class IFillRect;
class IScreen;

namespace Font5x7 {

enum class AlignX : uint8_t {
    Left,
    Center,
    Right,
};

int textWidth(const char* s, int scale);
int alignedTextX(int anchorX, const char* s, int scale, AlignX align);
bool textPixel(const char* s, int scale, int x, int y);
void drawText(int x, int y, const char* s, int scale, uint16_t color565, IFillRect& fillRect);
void drawTextBlock(int anchorX, int y, const char* s, int scale, uint16_t color565, AlignX align,
                   IFillRect& fillRect);
void drawCenteredText(IFillRect& fillRect, int areaWidth, int y, const char* s, int scale,
                      uint16_t color565);
void drawCenteredText(IScreen& screen, int y, const char* s, int scale, uint16_t color565);

}  // namespace Font5x7
