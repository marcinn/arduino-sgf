#include "BitTileCollider.h"

#include "ColliderCollision.h"
#include "Collision.h"
#include "CollisionShape.h"
#include "Physics.h"

#include <math.h>

namespace {
int clampTileIndex(int value, int limit) {
    if (value < 0) {
        return 0;
    }
    if (value >= limit) {
        return limit - 1;
    }
    return value;
}

bool bitAt(const uint8_t* tiles, const Vector2i& tileCount, int tileX, int tileY) {
    int index = tileY * tileCount.x + tileX;
    int byteIndex = index / 8;
    uint8_t bitMask = (uint8_t)(1u << (index % 8));
    return (tiles[byteIndex] & bitMask) != 0;
}

float min4(float a, float b, float c, float d) {
    float value = a;
    if (b < value) {
        value = b;
    }
    if (c < value) {
        value = c;
    }
    if (d < value) {
        value = d;
    }
    return value;
}

Vector2i floorVector(const Vector2f& value) {
    return Vector2i{(int)floorf(value.x), (int)floorf(value.y)};
}

Vector2i ceilVector(const Vector2f& value) {
    return Vector2i{(int)ceilf(value.x), (int)ceilf(value.y)};
}

bool shapeHitsRect(const Vector2f& position, const CollisionShape& collisionShape,
                   const Vector2i& rectMin, const Vector2i& rectMax) {
    Vector2i center = floorVector(position);
    if (collisionShape.type() == CollisionShapeType::Point) {
        return pointInRect(center, rectMin, rectMax);
    }
    if (collisionShape.type() == CollisionShapeType::Circle) {
        return circleRectHit(center, collisionShape.radius(), rectMin, rectMax);
    }

    Vector2f halfSize = Vector2f{collisionShape.size()} / 2.0f;
    Vector2i shapeMin = floorVector(position - halfSize);
    Vector2i shapeMax = ceilVector(position + halfSize);
    return aabbHit(shapeMin, shapeMax, rectMin, rectMax);
}
}  // namespace

BitTileCollider::BitTileCollider() = default;

BitTileCollider::BitTileCollider(const uint8_t* tiles, const Vector2i& tileCount,
                                 const Vector2i& tileSize, const Vector2f& minBounds,
                                 const Vector2f& maxBounds,
                                 float restitution)
    : tileBuffer(tiles), tileMapCount(tileCount), tileScreenSize(tileSize),
      minCollisionBounds(minBounds), maxCollisionBounds(maxBounds) {
    setRestitution(restitution);
}

BitTileCollider::BitTileCollider(const uint8_t* tiles, const Vector2i& tileCount,
                                 const Vector2i& tileSize, const Vector2i& minBounds,
                                 const Vector2i& maxBounds,
                                 float restitution)
    : BitTileCollider(tiles, tileCount, tileSize, Vector2f{minBounds}, Vector2f{maxBounds},
                      restitution) {}

const uint8_t* BitTileCollider::tiles() const { return tileBuffer; }

Vector2i BitTileCollider::tileCount() const { return tileMapCount; }

Vector2i BitTileCollider::tileSize() const { return tileScreenSize; }

Vector2f BitTileCollider::minBounds() const { return minCollisionBounds; }

Vector2f BitTileCollider::maxBounds() const { return maxCollisionBounds; }

float BitTileCollider::restitution() const { return collisionRestitution; }

void BitTileCollider::setTiles(const uint8_t* tiles) { tileBuffer = tiles; }

void BitTileCollider::setTileCount(const Vector2i& tileCount) { tileMapCount = tileCount; }

void BitTileCollider::setTileSize(const Vector2i& tileSize) { tileScreenSize = tileSize; }

void BitTileCollider::setBounds(const Vector2i& minBounds, const Vector2i& maxBounds) {
    setBounds(Vector2f{minBounds}, Vector2f{maxBounds});
}

void BitTileCollider::setBounds(const Vector2f& minBounds, const Vector2f& maxBounds) {
    minCollisionBounds = minBounds;
    maxCollisionBounds = maxBounds;
}

void BitTileCollider::setRestitution(float restitutionValue) {
    if (restitutionValue < 0.0f) {
        collisionRestitution = 0.0f;
    } else {
        collisionRestitution = restitutionValue;
    }
}

bool BitTileCollider::isColliding(const ICollidable& collidable) const {
    return getCollision(collidable).isColliding();
}

ColliderCollision BitTileCollider::getCollision(const ICollidable& collidable) const {
    ColliderCollision collision;
    Vector2f position = collidable.getCollisionPosition();
    CollisionShape collisionShape = collidable.getCollisionShape();
    if (!tileBuffer) {
        return collision;
    }
    if (tileMapCount.x <= 0 || tileMapCount.y <= 0) {
        return collision;
    }
    if (tileScreenSize.x <= 0 || tileScreenSize.y <= 0) {
        return collision;
    }

    Vector2f halfSpan = Vector2f{(float)tileScreenSize.x / 2.0f, (float)tileScreenSize.y / 2.0f};
    Vector2f searchMin = position - halfSpan;
    Vector2f searchMax = position + halfSpan;
    if (collisionShape.type() == CollisionShapeType::Rect) {
        searchMin -= Vector2f{collisionShape.size()} / 2.0f;
        searchMax += Vector2f{collisionShape.size()} / 2.0f;
    } else if (collisionShape.type() == CollisionShapeType::Circle) {
        float radius = (float)collisionShape.radius();
        searchMin -= Vector2f{radius, radius};
        searchMax += Vector2f{radius, radius};
    }

    if (searchMax.x < minCollisionBounds.x || searchMin.x > maxCollisionBounds.x ||
        searchMax.y < minCollisionBounds.y || searchMin.y > maxCollisionBounds.y) {
        return collision;
    }

    int minTileX = clampTileIndex((int)floorf((searchMin.x - minCollisionBounds.x) / tileScreenSize.x),
                                  tileMapCount.x);
    int maxTileX = clampTileIndex((int)floorf((searchMax.x - minCollisionBounds.x) / tileScreenSize.x),
                                  tileMapCount.x);
    int minTileY = clampTileIndex((int)floorf((searchMin.y - minCollisionBounds.y) / tileScreenSize.y),
                                  tileMapCount.y);
    int maxTileY = clampTileIndex((int)floorf((searchMax.y - minCollisionBounds.y) / tileScreenSize.y),
                                  tileMapCount.y);

    float bestDistance = 0.0f;
    bool found = false;

    for (int tileY = minTileY; tileY <= maxTileY; ++tileY) {
        for (int tileX = minTileX; tileX <= maxTileX; ++tileX) {
            if (!bitAt(tileBuffer, tileMapCount, tileX, tileY)) {
                continue;
            }

            int tileIndex = tileY * tileMapCount.x + tileX;
            int bufferIndex = tileIndex / 8;
            int bitIndex = tileIndex % 8;
            Vector2i rectMin = floorVector(Vector2f{
                minCollisionBounds.x + (tileX * tileScreenSize.x),
                minCollisionBounds.y + (tileY * tileScreenSize.y),
            });
            Vector2i rectMax =
                rectMin + tileScreenSize - Vector2i{1, 1};
            if (!shapeHitsRect(position, collisionShape, rectMin, rectMax)) {
                continue;
            }

            float tileMinX = (float)rectMin.x;
            float tileMaxX = (float)(rectMax.x + 1);
            float tileMinY = (float)rectMin.y;
            float tileMaxY = (float)(rectMax.y + 1);
            float leftDistance = fabsf(position.x - tileMinX);
            float rightDistance = fabsf(tileMaxX - position.x);
            float topDistance = fabsf(position.y - tileMinY);
            float bottomDistance = fabsf(tileMaxY - position.y);
            float minDistance = min4(leftDistance, rightDistance, topDistance, bottomDistance);

            if (!found || minDistance < bestDistance) {
                found = true;
                bestDistance = minDistance;
                collision.setColliding(true);
                collision.setTileX(tileX);
                collision.setTileY(tileY);
                collision.setTileIndex(tileIndex);
                collision.setBufferIndex(bufferIndex);
                collision.setBitIndex(bitIndex);
                collision.setTileValue(1);

                if (minDistance == leftDistance) {
                    collision.setPosition(Vector2f{tileMinX, position.y});
                    collision.setNormal(Vector2f{-1.0f, 0.0f});
                } else if (minDistance == rightDistance) {
                    collision.setPosition(Vector2f{tileMaxX, position.y});
                    collision.setNormal(Vector2f{1.0f, 0.0f});
                } else if (minDistance == topDistance) {
                    collision.setPosition(Vector2f{position.x, tileMinY});
                    collision.setNormal(Vector2f{0.0f, -1.0f});
                } else {
                    collision.setPosition(Vector2f{position.x, tileMaxY});
                    collision.setNormal(Vector2f{0.0f, 1.0f});
                }
            }
        }
    }

    return collision;
}

void BitTileCollider::resolve(RigidBody& body) const {
    ColliderCollision collision = getCollision(body);
    if (!collision.isColliding()) {
        return;
    }

    body.setPosition(collision.position());
    Physics::bounce(body, collision.normal(), collisionRestitution);
    body.setOnFloor(collision.normal() == Vector2f{0.0f, -1.0f});
}
