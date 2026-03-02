#pragma once

#include <string.h>

#include "SGF/Character.h"
#include "SGF/Font5x7.h"
#include "SGF/IFillRect.h"
#include "SGF/Renderer.h"

class TextSprite : public Character, public IFillRect {
   public:
    enum class AlignX : uint8_t {
        Left,
        Center,
        Right,
    };

    static constexpr int MAX_TEXT_LEN = 31;
    static constexpr int MAX_BITMAP_W = 160;
    static constexpr int MAX_BITMAP_H = 32;
    static constexpr uint16_t TRANSPARENT = 0x0000u;

    void bindSprite(Renderer2D::SpriteHandle sprite);
    void setText(const char* text);
    void setScale(int newScale);
    void setColor(uint16_t newColor565);
    void setAlignX(AlignX newAlignX);
    void fillRect565(int x0, int y0, int w, int h, uint16_t color565) override;
    int width() const;
    int height() const;

   private:
    void didSetPosition() override;

    void rebuildBitmap();
    void syncBoundSprite();

    Renderer2D::SpriteHandle boundSprite;
    AlignX alignX = AlignX::Left;
    int scale = 1;
    int bitmapW = 0;
    int bitmapH = 0;
    uint16_t color565 = 0xFFFFu;
    char text_[MAX_TEXT_LEN + 1] = {};
    uint16_t pixels565[MAX_BITMAP_W * MAX_BITMAP_H] = {};
};
