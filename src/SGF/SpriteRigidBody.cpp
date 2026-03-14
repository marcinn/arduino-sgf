#include "SpriteRigidBody.h"

namespace {
Vector2i spritePositionFromBody(const RigidBody& body) {
    return Vector2i{body.getPosition()};
}
}  // namespace

void SpriteRigidBody::bindSprite(Renderer2D::SpriteHandle spriteRef) {
    boundSpritePtr = spriteRef;
    configureBoundSprite(boundSpritePtr);
    boundSpritePtr.setActive(true);
    boundSpritePtr.setAnchor(anchor());
    boundSpritePtr.setPosition(spritePositionFromBody(*this));
}

void SpriteRigidBody::redrawSprite() {
    if (!boundSpritePtr.isBound()) {
        return;
    }
    boundSpritePtr.redraw();
}

void SpriteRigidBody::setAnchor(const Vector2f& newAnchor) {
    RigidBody::setAnchor(newAnchor);
    if (!boundSpritePtr.isBound()) {
        return;
    }

    boundSpritePtr.setAnchor(newAnchor);
    boundSpritePtr.setPosition(spritePositionFromBody(*this));
}

void SpriteRigidBody::setPosition(const Vector2f& position) {
    RigidBody::setPosition(position);
    if (!boundSpritePtr.isBound()) {
        return;
    }

    boundSpritePtr.setActive(true);
    boundSpritePtr.setPosition(spritePositionFromBody(*this));
}

Renderer2D::SpriteHandle& SpriteRigidBody::boundSprite() { return boundSpritePtr; }

const Renderer2D::SpriteHandle& SpriteRigidBody::boundSprite() const { return boundSpritePtr; }
