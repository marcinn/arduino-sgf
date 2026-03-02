#include "SpriteRigidBody.h"

void SpriteRigidBody::bindSprite(Renderer2D::SpriteHandle spriteRef) {
    boundSpritePtr = spriteRef;
    configureBoundSprite(boundSpritePtr);
    Vector2f pos = getPosition();
    boundSpritePtr.setActive(true);
    boundSpritePtr.setPosition(Vector2i{pos});
}

void SpriteRigidBody::redrawSprite() {
    if (!boundSpritePtr.isBound()) {
        return;
    }
    boundSpritePtr.redraw();
}

void SpriteRigidBody::setPosition(const Vector2f& position) {
    RigidBody::setPosition(position);
    if (!boundSpritePtr.isBound()) {
        return;
    }

    boundSpritePtr.setActive(true);
    boundSpritePtr.setPosition(Vector2i{position});
}

Renderer2D::SpriteHandle& SpriteRigidBody::boundSprite() { return boundSpritePtr; }

const Renderer2D::SpriteHandle& SpriteRigidBody::boundSprite() const { return boundSpritePtr; }
