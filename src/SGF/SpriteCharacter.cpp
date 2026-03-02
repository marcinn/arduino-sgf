#include "SpriteCharacter.h"

Vector2i SpriteCharacter::getSize() const { return size; }

void SpriteCharacter::setSize(int w, int h) {
    size = Vector2i{w, h};
    if (boundSpritePtr.isBound()) {
        configureBoundSprite(boundSpritePtr);
        Position pos = getPosition();
        boundSpritePtr.setActive(true);
        boundSpritePtr.setPosition(pos.x, pos.y);
    }
}

void SpriteCharacter::setSize(const Vector2i& newSize) { setSize(newSize.x, newSize.y); }

void SpriteCharacter::bindSprite(Renderer2D::SpriteHandle spriteRef) {
    boundSpritePtr = spriteRef;
    configureBoundSprite(boundSpritePtr);
    Position pos = getPosition();
    boundSpritePtr.setActive(true);
    boundSpritePtr.setPosition(pos.x, pos.y);
}

Renderer2D::SpriteHandle& SpriteCharacter::boundSprite() { return boundSpritePtr; }

const Renderer2D::SpriteHandle& SpriteCharacter::boundSprite() const { return boundSpritePtr; }

void SpriteCharacter::didSetPosition() {
    if (!boundSpritePtr.isBound()) {
        return;
    }
    Position pos = getPosition();
    boundSpritePtr.setActive(true);
    boundSpritePtr.setPosition(pos.x, pos.y);
}
