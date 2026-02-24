#pragma once

#include "SGF/Character.h"
#include "SGF/Sprites.h"

class SpriteCharacter : public Character {
public:
  Vector2 getSize() const {
    return size;
  }

  void setSize(int w, int h) {
    size = Vector2{w, h};
    if (boundSpritePtr) {
      configureBoundSprite(*boundSpritePtr);
      Position pos = getPosition();
      boundSpritePtr->active = true;
      boundSpritePtr->setPosition(pos.x, pos.y);
    }
  }

  void setSize(const Vector2& newSize) {
    setSize(newSize.x, newSize.y);
  }

  void bindSprite(SpriteLayer::Sprite& spriteRef) {
    boundSpritePtr = &spriteRef;
    configureBoundSprite(*boundSpritePtr);
    Position pos = getPosition();
    boundSpritePtr->active = true;
    boundSpritePtr->setPosition(pos.x, pos.y);
  }

protected:
  virtual void configureBoundSprite(SpriteLayer::Sprite& sprite) = 0;

  SpriteLayer::Sprite* boundSprite() {
    return boundSpritePtr;
  }

  const SpriteLayer::Sprite* boundSprite() const {
    return boundSpritePtr;
  }

private:
  void didSetPosition() override {
    if (!boundSpritePtr) {
      return;
    }
    Position pos = getPosition();
    boundSpritePtr->active = true;
    boundSpritePtr->setPosition(pos.x, pos.y);
  }

  SpriteLayer::Sprite* boundSpritePtr = nullptr;
  Vector2 size{};
};
