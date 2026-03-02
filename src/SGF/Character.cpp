#include "Character.h"

Character::~Character() = default;

Vector2i Character::getPosition() const { return Position{posX, posY}; }

void Character::setPosition(const Position& pos) { setPosition(pos.x, pos.y); }

void Character::setX(int newX) { setPosition(newX, posY); }

void Character::setY(int newY) { setPosition(posX, newY); }

void Character::setPosition(int newX, int newY) {
    posX = newX;
    posY = newY;
    didSetPosition();
}

void Character::didSetPosition() {}
