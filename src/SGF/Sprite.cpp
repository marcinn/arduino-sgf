#include "Sprite.h"

bool Sprite::isActive() const { return active; }

Vector2i Sprite::position() const { return spritePosition; }

int Sprite::width() const { return spriteWidth; }

int Sprite::height() const { return spriteHeight; }

const uint16_t* Sprite::pixels() const { return spritePixels; }

uint16_t Sprite::transparentColor() const { return transparent; }

SpriteScale Sprite::scale() const { return spriteScale; }

Vector2f Sprite::anchor() const { return spriteAnchor; }

void Sprite::setAnchor(const Vector2f& newAnchor) { spriteAnchor = newAnchor; }

void Sprite::setScale(SpriteScale newScale) {
    if (spriteScale == newScale) {
        return;
    }

    spriteScale = newScale;
    redraw();
}

void Sprite::setBitmap(const uint16_t* pixels, int width, int height) {
    bool changed = (spritePixels != pixels) || (spriteWidth != width) || (spriteHeight != height);
    spritePixels = pixels;
    spriteWidth = width;
    spriteHeight = height;
    if (changed) {
        redraw();
    }
}

void Sprite::setBitmap(const uint16_t* pixels, int width, int height, uint16_t transparentColor) {
    bool changed = (transparent != transparentColor);
    setBitmap(pixels, width, height);
    transparent = transparentColor;
    if (changed) {
        redraw();
    }
}

void Sprite::setTransparent(uint16_t transparentColor) {
    if (transparent == transparentColor) {
        return;
    }

    transparent = transparentColor;
    redraw();
}

void Sprite::setActive(bool enabled) {
    if (active == enabled) {
        return;
    }

    active = enabled;
    redraw();
}

void Sprite::setPosition(const Vector2i& newPosition) { spritePosition = newPosition; }

void Sprite::translate(const Vector2i& delta) { spritePosition += delta; }

void Sprite::redraw() { ++redrawRevision_; }

uint32_t Sprite::redrawRevision() const { return redrawRevision_; }
