#pragma once

#include "SGF/Character.h"
#include "SGF/Sprites.h"

class SpriteCharacter : public Character {
   public:
    Vector2i getSize() const;
    void setSize(int w, int h);
    void setSize(const Vector2i& newSize);
    void bindSprite(SpriteLayer::Sprite& spriteRef);

   protected:
    virtual void configureBoundSprite(SpriteLayer::Sprite& sprite) = 0;

    SpriteLayer::Sprite* boundSprite();
    const SpriteLayer::Sprite* boundSprite() const;

   private:
    void didSetPosition() override;

    SpriteLayer::Sprite* boundSpritePtr = nullptr;
    Vector2i size{};
};
