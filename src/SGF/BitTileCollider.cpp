#include "BitTileCollider.h"

#include "ColliderCollision.h"
#include "Collision.h"
#include "CollisionShape.h"
#include "Physics.h"

#include <math.h>

namespace {
constexpr float COLLISION_SEPARATION_EPSILON = 1.0f;

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

float collisionExtentAlongNormal(const CollisionShape& collisionShape, const Vector2f& normal) {
    if (collisionShape.type() == CollisionShapeType::Circle) {
        return (float)collisionShape.radius();
    }
    if (collisionShape.type() == CollisionShapeType::Rect) {
        Vector2f halfSize = Vector2f{collisionShape.size()} / 2.0f;
        if (normal.x != 0.0f) {
            return halfSize.x;
        }
        if (normal.y != 0.0f) {
            return halfSize.y;
        }
    }
    return 0.0f;
}

Vector2f resolvedCollisionCenter(const Vector2f& position, const CollisionShape& collisionShape,
                                 const Vector2f& normal, float tileMinX, float tileMaxX,
                                 float tileMinY, float tileMaxY) {
    float extent = collisionExtentAlongNormal(collisionShape, normal) + COLLISION_SEPARATION_EPSILON;
    if (normal.x < 0.0f) {
        return Vector2f{tileMinX - extent, position.y};
    }
    if (normal.x > 0.0f) {
        return Vector2f{tileMaxX + extent, position.y};
    }
    if (normal.y < 0.0f) {
        return Vector2f{position.x, tileMinY - extent};
    }
    if (normal.y > 0.0f) {
        return Vector2f{position.x, tileMaxY + extent};
    }
    return position;
}

Vector2f preferredCollisionNormal(const RigidBody& body, const ColliderCollision& collision,
                                  const Vector2f& minBounds, const Vector2i& tileSize) {
    if (collision.tileX() < 0 || collision.tileY() < 0) {
        return collision.normal();
    }

    Vector2f center = body.getCollisionPosition();
    CollisionShape collisionShape = body.getCollisionShape();
    float extentX = collisionShape.type() == CollisionShapeType::Rect
        ? collisionShape.size().x / 2.0f
        : collisionShape.type() == CollisionShapeType::Circle
            ? (float)collisionShape.radius()
            : 0.0f;
    float extentY = collisionShape.type() == CollisionShapeType::Rect
        ? collisionShape.size().y / 2.0f
        : collisionShape.type() == CollisionShapeType::Circle
            ? (float)collisionShape.radius()
            : 0.0f;

    float tileMinX = minBounds.x + (collision.tileX() * tileSize.x);
    float tileMaxX = tileMinX + tileSize.x;
    float tileMinY = minBounds.y + (collision.tileY() * tileSize.y);
    float tileMaxY = tileMinY + tileSize.y;

    float penLeft = center.x <= tileMinX ? (center.x + extentX) - tileMinX : -1.0f;
    float penRight = center.x >= tileMaxX ? tileMaxX - (center.x - extentX) : -1.0f;
    float penTop = center.y <= tileMinY ? (center.y + extentY) - tileMinY : -1.0f;
    float penBottom = center.y >= tileMaxY ? tileMaxY - (center.y - extentY) : -1.0f;

    float bestPen = -1.0f;
    Vector2f bestNormal = collision.normal();
    auto consider = [&](float penetration, const Vector2f& normal) {
        if (penetration < 0.0f) {
            return;
        }
        if (bestPen < 0.0f || penetration < bestPen) {
            bestPen = penetration;
            bestNormal = normal;
        }
    };

    consider(penLeft, Vector2f{-1.0f, 0.0f});
    consider(penRight, Vector2f{1.0f, 0.0f});
    consider(penTop, Vector2f{0.0f, -1.0f});
    consider(penBottom, Vector2f{0.0f, 1.0f});

    if (bestPen < 0.0f) {
        return collision.normal();
    }

    int tiedAxes = 0;
    if (penLeft >= 0.0f && fabsf(penLeft - bestPen) < 0.001f) {
        ++tiedAxes;
    }
    if (penRight >= 0.0f && fabsf(penRight - bestPen) < 0.001f) {
        ++tiedAxes;
    }
    if (penTop >= 0.0f && fabsf(penTop - bestPen) < 0.001f) {
        ++tiedAxes;
    }
    if (penBottom >= 0.0f && fabsf(penBottom - bestPen) < 0.001f) {
        ++tiedAxes;
    }

    Vector2f velocity = body.getVelocity();
    float absVx = fabsf(velocity.x);
    float absVy = fabsf(velocity.y);
    if (tiedAxes > 1) {
        if (absVx > absVy) {
            return Vector2f{velocity.x > 0.0f ? -1.0f : 1.0f, 0.0f};
        }
        if (absVy > absVx) {
            return Vector2f{0.0f, velocity.y > 0.0f ? -1.0f : 1.0f};
        }
    }

    return bestNormal;
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
                    collision.setNormal(Vector2f{-1.0f, 0.0f});
                    collision.setPosition(resolvedCollisionCenter(
                        position, collisionShape, collision.normal(), tileMinX, tileMaxX,
                        tileMinY, tileMaxY));
                } else if (minDistance == rightDistance) {
                    collision.setNormal(Vector2f{1.0f, 0.0f});
                    collision.setPosition(resolvedCollisionCenter(
                        position, collisionShape, collision.normal(), tileMinX, tileMaxX,
                        tileMinY, tileMaxY));
                } else if (minDistance == topDistance) {
                    collision.setNormal(Vector2f{0.0f, -1.0f});
                    collision.setPosition(resolvedCollisionCenter(
                        position, collisionShape, collision.normal(), tileMinX, tileMaxX,
                        tileMinY, tileMaxY));
                } else {
                    collision.setNormal(Vector2f{0.0f, 1.0f});
                    collision.setPosition(resolvedCollisionCenter(
                        position, collisionShape, collision.normal(), tileMinX, tileMaxX,
                        tileMinY, tileMaxY));
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

    Vector2f normal =
        preferredCollisionNormal(body, collision, minCollisionBounds, tileScreenSize);
    Vector2f collisionPosition = collision.position();
    if (collision.tileX() >= 0 && collision.tileY() >= 0) {
        float tileMinX = minCollisionBounds.x + (collision.tileX() * tileScreenSize.x);
        float tileMaxX = tileMinX + tileScreenSize.x;
        float tileMinY = minCollisionBounds.y + (collision.tileY() * tileScreenSize.y);
        float tileMaxY = tileMinY + tileScreenSize.y;
        collisionPosition = resolvedCollisionCenter(
            body.getCollisionPosition(), body.getCollisionShape(), normal,
            tileMinX, tileMaxX, tileMinY, tileMaxY);
    }

    Vector2f collisionOffset = collisionPosition - body.getCollisionPosition();
    body.setPosition(body.getPosition() + collisionOffset);
    Physics::bounce(body, normal, collisionRestitution);
    body.setOnFloor(normal == Vector2f{0.0f, -1.0f});
}
