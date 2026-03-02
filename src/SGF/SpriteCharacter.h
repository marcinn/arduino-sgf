#pragma once

#include "CharacterBody.h"
#include "Renderer.h"

class SpriteCharacter : public CharacterBody {
   public:
    Vector2i getSize() const;
    void setSize(int w, int h);
    void setSize(const Vector2i& newSize);
    void bindSprite(Renderer2D::SpriteHandle spriteRef);

   protected:
    virtual void configureBoundSprite(Renderer2D::SpriteHandle& sprite) = 0;

    Renderer2D::SpriteHandle& boundSprite();
    const Renderer2D::SpriteHandle& boundSprite() const;

   private:
    Renderer2D::SpriteHandle boundSpritePtr;
    Vector2i size{};
};
