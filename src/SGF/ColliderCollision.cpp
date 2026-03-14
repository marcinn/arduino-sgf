#include "ColliderCollision.h"

ColliderCollision::ColliderCollision() = default;

bool ColliderCollision::isColliding() const { return colliding; }

void ColliderCollision::setColliding(bool collidingValue) { colliding = collidingValue; }

Vector2f ColliderCollision::position() const { return collisionPosition; }

void ColliderCollision::setPosition(const Vector2i& positionValue) {
    setPosition(Vector2f{positionValue});
}

void ColliderCollision::setPosition(const Vector2f& positionValue) {
    collisionPosition = positionValue;
}

Vector2f ColliderCollision::normal() const { return collisionNormal; }

void ColliderCollision::setNormal(const Vector2i& normalValue) {
    setNormal(Vector2f{normalValue});
}

void ColliderCollision::setNormal(const Vector2f& normalValue) {
    collisionNormal = normalValue;
}

int ColliderCollision::tileX() const { return collisionTileX; }

void ColliderCollision::setTileX(int tileXValue) { collisionTileX = tileXValue; }

int ColliderCollision::tileY() const { return collisionTileY; }

void ColliderCollision::setTileY(int tileYValue) { collisionTileY = tileYValue; }

int ColliderCollision::tileIndex() const { return collisionTileIndex; }

void ColliderCollision::setTileIndex(int tileIndexValue) { collisionTileIndex = tileIndexValue; }

int ColliderCollision::bufferIndex() const { return collisionBufferIndex; }

void ColliderCollision::setBufferIndex(int bufferIndexValue) {
    collisionBufferIndex = bufferIndexValue;
}

int ColliderCollision::bitIndex() const { return collisionBitIndex; }

void ColliderCollision::setBitIndex(int bitIndexValue) { collisionBitIndex = bitIndexValue; }

uint8_t ColliderCollision::tileValue() const { return collisionTileValue; }

void ColliderCollision::setTileValue(uint8_t tileValueValue) {
    collisionTileValue = tileValueValue;
}
