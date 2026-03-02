#include "SpriteLayer.h"

Vector2i SpriteLayer::scaleFactor(SpriteScale scale) {
    switch (scale) {
        case SpriteScale::DoubleX:
            return Vector2i{2, 1};
        case SpriteScale::DoubleY:
            return Vector2i{1, 2};
        case SpriteScale::Double:
            return Vector2i{2, 2};
        case SpriteScale::QuadX:
            return Vector2i{4, 1};
        case SpriteScale::QuadY:
            return Vector2i{1, 4};
        case SpriteScale::Quad:
            return Vector2i{4, 4};
        case SpriteScale::Normal:
        default:
            return Vector2i{1, 1};
    }
}

Vector2i SpriteLayer::scaledSize(const Sprite& sprite) {
    return Vector2i{sprite.width(), sprite.height()} * scaleFactor(sprite.scale());
}

Vector2i SpriteLayer::anchorOffset(const Vector2f& anchor, const Vector2i& size) {
    Vector2i offset{};
    Vector2f sizeSpan = Vector2f{size - Vector2i{1, 1}};
    Vector2f value = anchor * sizeSpan;
    if (size.x > 0) {
        offset.x = (value.x >= 0.0f) ? (int)(value.x + 0.5f) : (int)(value.x - 0.5f);
    }
    if (size.y > 0) {
        offset.y = (value.y >= 0.0f) ? (int)(value.y + 0.5f) : (int)(value.y - 0.5f);
    }
    return offset;
}

Vector2i SpriteLayer::spriteTopLeft(const Sprite& sprite) {
    return sprite.position() - anchorOffset(sprite.anchor(), scaledSize(sprite));
}

SpriteLayer::SpriteLayer() = default;

void SpriteLayer::clearSprites() {
    for (auto& sprite : sprites_) {
        sprite.setActive(false);
    }
}

void SpriteLayer::clearMissiles() {
    for (auto& missile : missiles_) {
        missile.setActive(false);
    }
}

void SpriteLayer::clearAll() {
    clearSprites();
    clearMissiles();
}

Sprite& SpriteLayer::sprite(int index) {
    if (index < 0) {
        index = 0;
    }
    if (index >= MAX_SPRITES) {
        index = MAX_SPRITES - 1;
    }
    return sprites_[index];
}

Missile& SpriteLayer::missile(int index) {
    if (index < 0) {
        index = 0;
    }
    if (index >= MAX_MISSILES) {
        index = MAX_MISSILES - 1;
    }
    return missiles_[index];
}

void SpriteLayer::spriteBounds(const Sprite& sprite, int* x0, int* y0, int* x1, int* y1) {
    Vector2i topLeft = spriteTopLeft(sprite);
    Vector2i bottomRight = topLeft + scaledSize(sprite) - Vector2i{1, 1};

    if (x0) {
        *x0 = topLeft.x;
    }
    if (y0) {
        *y0 = topLeft.y;
    }
    if (x1) {
        *x1 = bottomRight.x;
    }
    if (y1) {
        *y1 = bottomRight.y;
    }
}

void SpriteLayer::spriteBounds(const Sprite& sprite, int16_t* x0, int16_t* y0, int16_t* x1,
                               int16_t* y1) {
    int left = 0;
    int top = 0;
    int right = 0;
    int bottom = 0;
    spriteBounds(sprite, &left, &top, &right, &bottom);
    if (x0) {
        *x0 = (int16_t)left;
    }
    if (y0) {
        *y0 = (int16_t)top;
    }
    if (x1) {
        *x1 = (int16_t)right;
    }
    if (y1) {
        *y1 = (int16_t)bottom;
    }
}

void SpriteLayer::spriteBoundsPadded(const Sprite& sprite, int pad, int* x0, int* y0, int* x1,
                                     int* y1) {
    int left = 0;
    int top = 0;
    int right = 0;
    int bottom = 0;
    spriteBounds(sprite, &left, &top, &right, &bottom);
    if (x0) {
        *x0 = left - pad;
    }
    if (y0) {
        *y0 = top - pad;
    }
    if (x1) {
        *x1 = right + pad;
    }
    if (y1) {
        *y1 = bottom + pad;
    }
}

void SpriteLayer::renderRegion(int x0, int y0, int w, int h, uint16_t* buf) const {
    if (!buf || w <= 0 || h <= 0) {
        return;
    }

    auto blitMissile = [&](const Missile& missile) {
        if (!missile.isActive()) {
            return;
        }

        Vector2i missilePosition = missile.position();
        Vector2i missileSize = missile.size();
        if (missileSize.x <= 0 || missileSize.y <= 0) {
            return;
        }

        Vector2i scale = scaleFactor(missile.scale());
        int missileX0 = missilePosition.x;
        int missileY0 = missilePosition.y;
        int scaledW = missileSize.x * scale.x;
        int scaledH = missileSize.y * scale.y;
        int missileX1 = missilePosition.x + scaledW - 1;
        int missileY1 = missilePosition.y + scaledH - 1;
        if (missileX1 < x0 || missileX0 >= x0 + w || missileY1 < y0 || missileY0 >= y0 + h) {
            return;
        }

        int regionX0 = (missileX0 < x0) ? x0 : missileX0;
        int regionY0 = (missileY0 < y0) ? y0 : missileY0;
        int regionX1 = (missileX1 > x0 + w - 1) ? (x0 + w - 1) : missileX1;
        int regionY1 = (missileY1 > y0 + h - 1) ? (y0 + h - 1) : missileY1;

        for (int yy = regionY0; yy <= regionY1; ++yy) {
            for (int xx = regionX0; xx <= regionX1; ++xx) {
                int srcX = xx - missileX0;
                if (scale.x > 1) {
                    srcX /= scale.x;
                }
                if (srcX < 0 || srcX >= missileSize.x) {
                    continue;
                }

                int bufX = xx - x0;
                int bufY = yy - y0;
                buf[bufY * w + bufX] = missile.color();
            }
        }
    };

    auto blitSprite = [&](const Sprite& sprite) {
        if (!sprite.isActive() || !sprite.pixels() || sprite.width() <= 0 || sprite.height() <= 0) {
            return;
        }

        Vector2i scale = scaleFactor(sprite.scale());
        int spriteX0 = 0;
        int spriteY0 = 0;
        int spriteX1 = 0;
        int spriteY1 = 0;
        spriteBounds(sprite, &spriteX0, &spriteY0, &spriteX1, &spriteY1);
        if (spriteX1 < x0 || spriteX0 >= x0 + w || spriteY1 < y0 || spriteY0 >= y0 + h) {
            return;
        }

        int regionX0 = (spriteX0 < x0) ? x0 : spriteX0;
        int regionY0 = (spriteY0 < y0) ? y0 : spriteY0;
        int regionX1 = (spriteX1 > x0 + w - 1) ? (x0 + w - 1) : spriteX1;
        int regionY1 = (spriteY1 > y0 + h - 1) ? (y0 + h - 1) : spriteY1;

        for (int yy = regionY0; yy <= regionY1; ++yy) {
            int srcY = yy - spriteY0;
            if (scale.y > 1) {
                srcY /= scale.y;
            }
            for (int xx = regionX0; xx <= regionX1; ++xx) {
                int srcX = xx - spriteX0;
                if (scale.x > 1) {
                    srcX /= scale.x;
                }
                if (srcX < 0 || srcX >= sprite.width()) {
                    continue;
                }

                const uint16_t color = sprite.pixels()[srcY * sprite.width() + srcX];
                if (color == sprite.transparentColor()) {
                    continue;
                }

                int bufX = xx - x0;
                int bufY = yy - y0;
                buf[bufY * w + bufX] = color;
            }
        }
    };

    for (const auto& missile : missiles_) {
        blitMissile(missile);
    }
    for (const auto& sprite : sprites_) {
        blitSprite(sprite);
    }
}
