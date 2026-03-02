#include "CharacterBody.h"

CharacterBody::~CharacterBody() = default;

Vector2i CharacterBody::getPosition() const { return Position{posX, posY}; }

void CharacterBody::setPosition(const Position& pos) { setPosition(pos.x, pos.y); }

void CharacterBody::setPosition(int newX, int newY) {
    posX = newX;
    posY = newY;
}

Vector2f CharacterBody::getCollisionPosition() const { return Vector2f{getPosition()}; }

CollisionShape CharacterBody::getCollisionShape() const { return collisionShape; }

void CharacterBody::setCollisionShape(const CollisionShape& collisionShapeValue) {
    collisionShape = collisionShapeValue;
}
