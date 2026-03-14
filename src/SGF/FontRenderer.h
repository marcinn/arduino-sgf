#pragma once

#include <stdint.h>

class IFillRect;
class IFont;
class IScreen;

class FontRenderer {
   public:
    enum class AlignX : uint8_t {
        Left,
        Center,
        Right,
    };

    static int textWidth(const IFont& font, const char* text, int scale);
    static void drawText(const IFont& font,
                         IFillRect& fillRect,
                         int x,
                         int y,
                         const char* text,
                         int scale,
                         uint16_t color565);
    static void drawText(const IFont& font,
                         IScreen& screen,
                         int x,
                         int y,
                         const char* text,
                         int scale,
                         uint16_t color565);
    static void drawTextBlock(const IFont& font,
                              IFillRect& fillRect,
                              int anchorX,
                              int y,
                              const char* text,
                              int scale,
                              uint16_t color565,
                              AlignX align);
    static void drawTextBlock(const IFont& font,
                              IScreen& screen,
                              int anchorX,
                              int y,
                              const char* text,
                              int scale,
                              uint16_t color565,
                              AlignX align);
    static void drawTextCentered(const IFont& font,
                                 IFillRect& fillRect,
                                 int centerX,
                                 int y,
                                 const char* text,
                                 int scale,
                                 uint16_t color565);
    static void drawTextCentered(const IFont& font,
                                 IScreen& screen,
                                 int centerX,
                                 int y,
                                 const char* text,
                                 int scale,
                                 uint16_t color565);
};
