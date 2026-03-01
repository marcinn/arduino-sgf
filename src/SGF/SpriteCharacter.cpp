#include "SpriteCharacter.h"

Vector2 SpriteCharacter::getSize() const {
  return size;
}

void SpriteCharacter::setSize(int w, int h) {
  size = Vector2{w, h};
  if (boundSpritePtr) {
    configureBoundSprite(*boundSpritePtr);
    Position pos = getPosition();
    boundSpritePtr->active = true;
    boundSpritePtr->setPosition(pos.x, pos.y);
  }
}

void SpriteCharacter::setSize(const Vector2& newSize) {
  setSize(newSize.x, newSize.y);
}

void SpriteCharacter::bindSprite(SpriteLayer::Sprite& spriteRef) {
  boundSpritePtr = &spriteRef;
  configureBoundSprite(*boundSpritePtr);
  Position pos = getPosition();
  boundSpritePtr->active = true;
  boundSpritePtr->setPosition(pos.x, pos.y);
}

SpriteLayer::Sprite* SpriteCharacter::boundSprite() {
  return boundSpritePtr;
}

const SpriteLayer::Sprite* SpriteCharacter::boundSprite() const {
  return boundSpritePtr;
}

void SpriteCharacter::didSetPosition() {
  if (!boundSpritePtr) {
    return;
  }
  Position pos = getPosition();
  boundSpritePtr->active = true;
  boundSpritePtr->setPosition(pos.x, pos.y);
}
