#pragma once

#include <string.h>

#include "SGF/Character.h"
#include "SGF/Font5x7.h"
#include "SGF/Sprites.h"

class TextSprite : public Character {
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

  void bindSprite(SpriteLayer::Sprite& sprite) {
    boundSprite = &sprite;
    syncBoundSprite();
  }

  void setText(const char* text) {
    const char* nextText = text ? text : "";
    if (strncmp(text_, nextText, MAX_TEXT_LEN) == 0 && strlen(text_) == strlen(nextText)) {
      return;
    }

    strncpy(text_, nextText, MAX_TEXT_LEN);
    text_[MAX_TEXT_LEN] = '\0';
    rebuildBitmap();
  }

  void setScale(int newScale) {
    int nextScale = newScale > 0 ? newScale : 1;
    if (scale == nextScale) {
      return;
    }

    scale = nextScale;
    rebuildBitmap();
  }

  void setColor(uint16_t newColor565) {
    if (color565 == newColor565) {
      return;
    }

    color565 = newColor565;
    rebuildBitmap();
  }

  void setAlignX(AlignX newAlignX) {
    if (alignX == newAlignX) {
      return;
    }

    alignX = newAlignX;
    syncBoundSprite();
  }

  int width() const {
    return bitmapW;
  }

  int height() const {
    return bitmapH;
  }

private:
  static void fillRectInBitmap(void* ctx, int x, int y, int w, int h, uint16_t color565) {
    TextSprite& sprite = *static_cast<TextSprite*>(ctx);
    if (w <= 0 || h <= 0 || sprite.bitmapW <= 0 || sprite.bitmapH <= 0) {
      return;
    }

    int x1 = x + w;
    int y1 = y + h;
    int clipX0 = x < 0 ? 0 : x;
    int clipY0 = y < 0 ? 0 : y;
    int clipX1 = x1 > sprite.bitmapW ? sprite.bitmapW : x1;
    int clipY1 = y1 > sprite.bitmapH ? sprite.bitmapH : y1;
    if (clipX0 >= clipX1 || clipY0 >= clipY1) {
      return;
    }

    for (int yy = clipY0; yy < clipY1; ++yy) {
      uint16_t* row = sprite.pixels565 + yy * sprite.bitmapW;
      for (int xx = clipX0; xx < clipX1; ++xx) {
        row[xx] = color565;
      }
    }
  }

  void didSetPosition() override {
    syncBoundSprite();
  }

  void rebuildBitmap() {
    bitmapW = Font5x7::textWidth(text_, scale);
    bitmapH = text_[0] != '\0' ? 7 * scale : 0;
    if (bitmapW > MAX_BITMAP_W) {
      bitmapW = MAX_BITMAP_W;
    }
    if (bitmapH > MAX_BITMAP_H) {
      bitmapH = MAX_BITMAP_H;
    }

    const int totalPixels = bitmapW * bitmapH;
    for (int i = 0; i < totalPixels; ++i) {
      pixels565[i] = TRANSPARENT;
    }

    if (bitmapW > 0 && bitmapH > 0) {
      Font5x7::drawText(0, 0, text_, scale, color565, this, fillRectInBitmap);
    }
    syncBoundSprite();
  }

  void syncBoundSprite() {
    if (!boundSprite) {
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

    boundSprite->setAnchor(anchorX, 0.0f);
    boundSprite->setBitmap(pixels565, bitmapW > 0 ? bitmapW : 1, bitmapH > 0 ? bitmapH : 1, TRANSPARENT);
    boundSprite->setPosition(getPosition().x, getPosition().y);
    boundSprite->setActive(bitmapW > 0 && bitmapH > 0);
  }

  SpriteLayer::Sprite* boundSprite = nullptr;
  AlignX alignX = AlignX::Left;
  int scale = 1;
  int bitmapW = 0;
  int bitmapH = 0;
  uint16_t color565 = 0xFFFFu;
  char text_[MAX_TEXT_LEN + 1] = {};
  uint16_t pixels565[MAX_BITMAP_W * MAX_BITMAP_H] = {};
};
