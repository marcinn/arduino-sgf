#include "SpriteCharacter.h"

namespace {
Vector2i spritePositionFromCharacter(const CharacterBody& body) {
    return Vector2i{body.getPosition()};
}
}  // namespace

Vector2i SpriteCharacter::getSize() const { return size; }

void SpriteCharacter::setSize(int w, int h) {
    size = Vector2i{w, h};
    if (boundSpritePtr.isBound()) {
        configureBoundSprite(boundSpritePtr);
        boundSpritePtr.setActive(true);
        boundSpritePtr.setAnchor(anchor());
        boundSpritePtr.setPosition(spritePositionFromCharacter(*this));
    }
}

void SpriteCharacter::setSize(const Vector2i& newSize) { setSize(newSize.x, newSize.y); }

void SpriteCharacter::setPosition(const Position& pos) { setPosition(pos.x, pos.y); }

void SpriteCharacter::setPosition(int newX, int newY) {
    CharacterBody::setPosition(newX, newY);
    if (!boundSpritePtr.isBound()) {
        return;
    }

    boundSpritePtr.setActive(true);
    boundSpritePtr.setPosition(spritePositionFromCharacter(*this));
}

void SpriteCharacter::setAnchor(const Vector2f& newAnchor) {
    CharacterBody::setAnchor(newAnchor);
    if (!boundSpritePtr.isBound()) {
        return;
    }

    boundSpritePtr.setAnchor(newAnchor);
    boundSpritePtr.setPosition(spritePositionFromCharacter(*this));
}

void SpriteCharacter::bindSprite(Renderer2D::SpriteHandle spriteRef) {
    boundSpritePtr = spriteRef;
    configureBoundSprite(boundSpritePtr);
    boundSpritePtr.setActive(true);
    boundSpritePtr.setAnchor(anchor());
    boundSpritePtr.setPosition(spritePositionFromCharacter(*this));
}

Renderer2D::SpriteHandle& SpriteCharacter::boundSprite() { return boundSpritePtr; }

const Renderer2D::SpriteHandle& SpriteCharacter::boundSprite() const { return boundSpritePtr; }
