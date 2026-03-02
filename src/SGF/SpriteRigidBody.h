#pragma once

#include "Renderer2D.h"
#include "RigidBody.h"

class SpriteRigidBody : public RigidBody {
   public:
    using RigidBody::setPosition;

    void bindSprite(Renderer2D::SpriteHandle spriteRef);
    void redrawSprite();
    void setAnchor(const Vector2f& newAnchor) override;
    void setPosition(const Vector2f& position) override;

   protected:
    virtual void configureBoundSprite(Renderer2D::SpriteHandle& sprite) = 0;

    Renderer2D::SpriteHandle& boundSprite();
    const Renderer2D::SpriteHandle& boundSprite() const;

   private:
    Renderer2D::SpriteHandle boundSpritePtr;
};
