#include "CharacterBody.h"

CharacterBody::~CharacterBody() = default;

Vector2i CharacterBody::getPosition() const { return Position{posX, posY}; }

void CharacterBody::setPosition(const Position& pos) { setPosition(pos.x, pos.y); }

void CharacterBody::setPosition(int newX, int newY) {
    posX = newX;
    posY = newY;
}

Vector2f CharacterBody::anchor() const { return bodyAnchor; }

void CharacterBody::setAnchor(const Vector2f& newAnchor) { bodyAnchor = newAnchor; }

Vector2f CharacterBody::getCollisionPosition() const {
    Vector2f position = Vector2f{getPosition()};
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

CollisionShape CharacterBody::getCollisionShape() const { return collisionShape; }

void CharacterBody::setCollisionShape(const CollisionShape& collisionShapeValue) {
    collisionShape = collisionShapeValue;
}
