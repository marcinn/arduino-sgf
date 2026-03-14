#include "Missile.h"

bool Missile::isActive() const { return active; }

Vector2i Missile::position() const { return missilePosition; }

Vector2i Missile::size() const { return missileSize; }

uint16_t Missile::color() const { return missileColor; }

SpriteScale Missile::scale() const { return missileScale; }

void Missile::setActive(bool enabled) { active = enabled; }

void Missile::setPosition(const Vector2i& newPosition) { missilePosition = newPosition; }

void Missile::setSize(const Vector2i& newSize) { missileSize = newSize; }

void Missile::setColor(uint16_t newColor) { missileColor = newColor; }

void Missile::setScale(SpriteScale newScale) { missileScale = newScale; }
