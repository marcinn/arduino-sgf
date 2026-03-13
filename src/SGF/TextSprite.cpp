#include "TextSprite.h"

#include <stdlib.h>

#include "FontRenderer.h"

namespace {
uint16_t kEmptyTextSpritePixel = TextSprite::TRANSPARENT;
}

TextSprite::TextSprite() = default;

TextSprite::TextSprite(const TextSprite& other) { copyFrom(other); }

TextSprite::TextSprite(TextSprite&& other) noexcept { moveFrom(static_cast<TextSprite&&>(other)); }

TextSprite::~TextSprite() {
    releaseBitmapStorage();
}

TextSprite& TextSprite::operator=(const TextSprite& other) {
    if (this != &other) {
        releaseBitmapStorage();
        copyFrom(other);
    }
    return *this;
}

TextSprite& TextSprite::operator=(TextSprite&& other) noexcept {
    if (this != &other) {
        releaseBitmapStorage();
        moveFrom(static_cast<TextSprite&&>(other));
    }
    return *this;
}

void TextSprite::releaseBitmapStorage() {
#if SGF_TEXTSPRITE_HEAP_BITMAP
    free(pixels565);
    pixels565 = nullptr;
#endif
}

uint16_t* TextSprite::bitmapPixels() { return pixels565; }

const uint16_t* TextSprite::bitmapPixels() const { return pixels565; }

bool TextSprite::ensureBitmapStorage() {
#if SGF_TEXTSPRITE_HEAP_BITMAP
    if (pixels565 == nullptr) {
        pixels565 = static_cast<uint16_t*>(malloc(sizeof(uint16_t) * MAX_BITMAP_W * MAX_BITMAP_H));
    }
#endif
    return pixels565 != nullptr;
}

void TextSprite::copyFrom(const TextSprite& other) {
    boundSprite = other.boundSprite;
    alignX = other.alignX;
    scale = other.scale;
    bitmapW = other.bitmapW;
    bitmapH = other.bitmapH;
    color565 = other.color565;
    memcpy(text_, other.text_, sizeof(text_));
#if SGF_TEXTSPRITE_HEAP_BITMAP
    pixels565 = nullptr;
    if (other.pixels565 != nullptr && ensureBitmapStorage()) {
        memcpy(pixels565, other.pixels565, sizeof(uint16_t) * MAX_BITMAP_W * MAX_BITMAP_H);
    }
#else
    memcpy(pixels565Storage, other.pixels565Storage, sizeof(pixels565Storage));
    pixels565 = pixels565Storage;
#endif
}

void TextSprite::moveFrom(TextSprite&& other) {
    boundSprite = other.boundSprite;
    alignX = other.alignX;
    scale = other.scale;
    bitmapW = other.bitmapW;
    bitmapH = other.bitmapH;
    color565 = other.color565;
    memcpy(text_, other.text_, sizeof(text_));
#if SGF_TEXTSPRITE_HEAP_BITMAP
    pixels565 = other.pixels565;
    other.pixels565 = nullptr;
#else
    memcpy(pixels565Storage, other.pixels565Storage, sizeof(pixels565Storage));
    pixels565 = pixels565Storage;
#endif
}

void TextSprite::bindSprite(Renderer2D::SpriteHandle sprite) {
    boundSprite = sprite;
    syncBoundSprite();
}

void TextSprite::setText(const char* text) {
    const char* nextText = text ? text : "";
    if (strncmp(text_, nextText, MAX_TEXT_LEN) == 0 && strlen(text_) == strlen(nextText)) {
        return;
    }

    strncpy(text_, nextText, MAX_TEXT_LEN);
    text_[MAX_TEXT_LEN] = '\0';
    rebuildBitmap();
}

void TextSprite::setScale(int newScale) {
    int nextScale = newScale > 0 ? newScale : 1;
    if (scale == nextScale) {
        return;
    }

    scale = nextScale;
    rebuildBitmap();
}

void TextSprite::setColor(uint16_t newColor565) {
    if (color565 == newColor565) {
        return;
    }

    color565 = newColor565;
    rebuildBitmap();
}

void TextSprite::setAlignX(AlignX newAlignX) {
    if (alignX == newAlignX) {
        return;
    }

    alignX = newAlignX;
    syncBoundSprite();
}

int TextSprite::width() const { return bitmapW; }

int TextSprite::height() const { return bitmapH; }

void TextSprite::fillRect565(int x, int y, int w, int h, uint16_t color565) {
    uint16_t* pixels = bitmapPixels();
    if (pixels == nullptr || w <= 0 || h <= 0 || bitmapW <= 0 || bitmapH <= 0) {
        return;
    }

    int x1 = x + w;
    int y1 = y + h;
    int clipX0 = x < 0 ? 0 : x;
    int clipY0 = y < 0 ? 0 : y;
    int clipX1 = x1 > bitmapW ? bitmapW : x1;
    int clipY1 = y1 > bitmapH ? bitmapH : y1;
    if (clipX0 >= clipX1 || clipY0 >= clipY1) {
        return;
    }

    for (int yy = clipY0; yy < clipY1; ++yy) {
        uint16_t* row = pixels + yy * bitmapW;
        for (int xx = clipX0; xx < clipX1; ++xx) {
            row[xx] = color565;
        }
    }
}

void TextSprite::rebuildBitmap() {
    bitmapW = FontRenderer::textWidth(FONT_5X7, text_, scale);
    bitmapH = text_[0] != '\0' ? FONT_5X7.glyphHeight() * scale : 0;
    if (bitmapW > MAX_BITMAP_W) {
        bitmapW = MAX_BITMAP_W;
    }
    if (bitmapH > MAX_BITMAP_H) {
        bitmapH = MAX_BITMAP_H;
    }

    if (bitmapW > 0 && bitmapH > 0 && !ensureBitmapStorage()) {
        bitmapW = 0;
        bitmapH = 0;
        syncBoundSprite();
        return;
    }

    uint16_t* pixels = bitmapPixels();
    const int totalPixels = bitmapW * bitmapH;
    for (int i = 0; i < totalPixels; ++i) {
        pixels[i] = TRANSPARENT;
    }

    if (bitmapW > 0 && bitmapH > 0) {
        FontRenderer::drawText(FONT_5X7, *this, 0, 0, text_, scale, color565);
    }
    syncBoundSprite();
}

void TextSprite::syncBoundSprite() {
    if (!boundSprite.isBound()) {
        return;
    }

    float anchorX = 0.0f;
    switch (alignX) {
        case AlignX::Center:
            anchorX = 0.5f;
            break;
        case AlignX::Right:
            anchorX = 1.0f;
            break;
        case AlignX::Left:
        default:
            anchorX = 0.0f;
            break;
    }

    boundSprite.setAnchor(Vector2f{anchorX, 0.0f});
    const uint16_t* pixels = bitmapPixels();
    if (pixels == nullptr) {
        pixels = &kEmptyTextSpritePixel;
    }
    boundSprite.setBitmap(pixels, bitmapW > 0 ? bitmapW : 1, bitmapH > 0 ? bitmapH : 1, TRANSPARENT);
    boundSprite.setPosition(getPosition());
    boundSprite.setActive(bitmapW > 0 && bitmapH > 0);
}
