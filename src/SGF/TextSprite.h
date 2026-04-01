#pragma once

#include <string.h>

#ifndef SGF_TEXTSPRITE_HEAP_BITMAP
#define SGF_TEXTSPRITE_HEAP_BITMAP 1
#endif

#include "CharacterBody.h"
#include "Font5x7.h"
#include "IFillRect.h"
#include "Renderer2D.h"

class TextSprite : public CharacterBody, public IFillRect {
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

    TextSprite();
    TextSprite(const TextSprite& other);
    TextSprite(TextSprite&& other) noexcept;
    ~TextSprite();

    TextSprite& operator=(const TextSprite& other);
    TextSprite& operator=(TextSprite&& other) noexcept;

    void bindSprite(Renderer2D::SpriteHandle sprite);
    void setText(const char* text);
    void setScale(int newScale);
    void setColor(uint16_t newColor565);
    void setAlignX(AlignX newAlignX);
    void fillRect565(int x0, int y0, int w, int h, uint16_t color565) override;
    int width() const;
    int height() const;

   private:
    void rebuildBitmap();
    void syncBoundSprite();
    uint16_t* bitmapPixels();
    const uint16_t* bitmapPixels() const;
    bool ensureBitmapStorage();
    void releaseBitmapStorage();
    void copyFrom(const TextSprite& other);
    void moveFrom(TextSprite&& other);

    Renderer2D::SpriteHandle boundSprite;
    AlignX alignX = AlignX::Left;
    int scale = 1;
    int bitmapW = 0;
    int bitmapH = 0;
    uint16_t color565 = 0xFFFFu;
    char text_[MAX_TEXT_LEN + 1] = {};
#if SGF_TEXTSPRITE_HEAP_BITMAP
    uint16_t* pixels565 = nullptr;
#else
    uint16_t pixels565Storage[MAX_BITMAP_W * MAX_BITMAP_H] = {};
    uint16_t* pixels565 = pixels565Storage;
#endif
};
