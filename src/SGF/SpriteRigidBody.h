#pragma once

#include "SGF/Renderer.h"
#include "SGF/RigidBody.h"

class SpriteRigidBody : public RigidBody {
   public:
    void bindSprite(Renderer2D::SpriteHandle spriteRef);
    void redrawSprite();

   protected:
    virtual void configureBoundSprite(Renderer2D::SpriteHandle& sprite) = 0;

    Renderer2D::SpriteHandle& boundSprite();
    const Renderer2D::SpriteHandle& boundSprite() const;

   private:
    void didSetPosition() override;

    Renderer2D::SpriteHandle boundSpritePtr;
};
