#include "SpriteRigidBody.h"

void SpriteRigidBody::bindSprite(Renderer2D::SpriteHandle spriteRef) {
    boundSpritePtr = spriteRef;
    configureBoundSprite(boundSpritePtr);
    Position pos = getPosition();
    boundSpritePtr.setActive(true);
    boundSpritePtr.setPosition(pos.x, pos.y);
}

void SpriteRigidBody::redrawSprite() {
    if (!boundSpritePtr.isBound()) {
        return;
    }
    boundSpritePtr.redraw();
}

Renderer2D::SpriteHandle& SpriteRigidBody::boundSprite() { return boundSpritePtr; }

const Renderer2D::SpriteHandle& SpriteRigidBody::boundSprite() const { return boundSpritePtr; }

void SpriteRigidBody::didSetPosition() {
    RigidBody::didSetPosition();
    if (!boundSpritePtr.isBound()) {
        return;
    }
    Position pos = getPosition();
    boundSpritePtr.setActive(true);
    boundSpritePtr.setPosition(pos.x, pos.y);
}
