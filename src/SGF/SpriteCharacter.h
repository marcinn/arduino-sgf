#pragma once

#include "CharacterBody.h"
#include "Renderer2D.h"

class SpriteCharacter : public CharacterBody {
   public:
    Vector2i getSize() const;
    void setSize(int w, int h);
    void setSize(const Vector2i& newSize);
    void setPosition(const Position& pos) override;
    void setPosition(int newX, int newY) override;
    void setAnchor(const Vector2f& newAnchor) override;
    void bindSprite(Renderer2D::SpriteHandle spriteRef);

   protected:
    virtual void configureBoundSprite(Renderer2D::SpriteHandle& sprite) = 0;

    Renderer2D::SpriteHandle& boundSprite();
    const Renderer2D::SpriteHandle& boundSprite() const;

   private:
    Renderer2D::SpriteHandle boundSpritePtr;
    Vector2i size{};
};
