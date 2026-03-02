#include "StaticBody.h"

CollisionBodyType StaticBody::collisionBodyType() const { return CollisionBodyType::Static; }

Vector2f StaticBody::getPosition() const { return position; }

void StaticBody::setPosition(const Vector2i& position) { setPosition(Vector2f{position}); }

void StaticBody::setPosition(const Vector2f& position) { this->position = position; }

Vector2f StaticBody::anchor() const { return bodyAnchor; }

void StaticBody::setAnchor(const Vector2f& newAnchor) { bodyAnchor = newAnchor; }

Vector2f StaticBody::getCollisionPosition() const {
    if (collisionShape.type() == CollisionShapeType::Point) {
        return position;
    }

    Vector2f size{};
    if (collisionShape.type() == CollisionShapeType::Rect) {
        size = Vector2f{collisionShape.size()};
    } else if (collisionShape.type() == CollisionShapeType::Circle) {
        float diameter = (float)(collisionShape.radius() * 2);
        size = Vector2f{diameter, diameter};
    }

    return position + ((Vector2f{0.5f, 0.5f} - bodyAnchor) * size);
}

CollisionShape StaticBody::getCollisionShape() const { return collisionShape; }

void StaticBody::setCollisionShape(const CollisionShape& collisionShapeValue) {
    collisionShape = collisionShapeValue;
}
