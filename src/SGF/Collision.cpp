#include "Collision.h"

#include "CollisionShape.h"

#include <math.h>

namespace {
Vector2i floorVector(const Vector2f& value) {
    return Vector2i{(int)floorf(value.x), (int)floorf(value.y)};
}

Vector2i ceilVector(const Vector2f& value) {
    return Vector2i{(int)ceilf(value.x), (int)ceilf(value.y)};
}

Vector2i rectMinFor(const ICollidable& collidable) {
    Vector2f center = collidable.getCollisionPosition();
    CollisionShape shape = collidable.getCollisionShape();
    if (shape.type() == CollisionShapeType::Rect) {
        return floorVector(center - (Vector2f{shape.size()} / 2.0f));
    }
    if (shape.type() == CollisionShapeType::Circle) {
        return floorVector(center - Vector2f{(float)shape.radius(), (float)shape.radius()});
    }
    return floorVector(center);
}

Vector2i rectMaxFor(const ICollidable& collidable) {
    Vector2f center = collidable.getCollisionPosition();
    CollisionShape shape = collidable.getCollisionShape();
    if (shape.type() == CollisionShapeType::Rect) {
        return ceilVector(center + (Vector2f{shape.size()} / 2.0f)) - Vector2i{1, 1};
    }
    if (shape.type() == CollisionShapeType::Circle) {
        float radius = (float)shape.radius();
        return ceilVector(center + Vector2f{radius, radius}) - Vector2i{1, 1};
    }
    return floorVector(center);
}

Vector2f normalFromRects(const ICollidable& first, const ICollidable& second) {
    Vector2f delta = first.getCollisionPosition() - second.getCollisionPosition();
    Vector2i firstMin = rectMinFor(first);
    Vector2i firstMax = rectMaxFor(first);
    Vector2i secondMin = rectMinFor(second);
    Vector2i secondMax = rectMaxFor(second);

    float overlapX = (float)(firstMax.x >= secondMax.x ? secondMax.x - firstMin.x + 1
                                                       : firstMax.x - secondMin.x + 1);
    float overlapY = (float)(firstMax.y >= secondMax.y ? secondMax.y - firstMin.y + 1
                                                       : firstMax.y - secondMin.y + 1);
    if (overlapX <= overlapY) {
        return Vector2f{delta.x < 0.0f ? -1.0f : 1.0f, 0.0f};
    }
    return Vector2f{0.0f, delta.y < 0.0f ? -1.0f : 1.0f};
}

Vector2f normalFromPointToRect(const Vector2f& point, const Vector2i& rectMin,
                               const Vector2i& rectMax) {
    float leftDistance = fabsf(point.x - rectMin.x);
    float rightDistance = fabsf((rectMax.x + 1) - point.x);
    float topDistance = fabsf(point.y - rectMin.y);
    float bottomDistance = fabsf((rectMax.y + 1) - point.y);
    float minDistance = leftDistance;
    Vector2f normal{-1.0f, 0.0f};

    if (rightDistance < minDistance) {
        minDistance = rightDistance;
        normal = Vector2f{1.0f, 0.0f};
    }
    if (topDistance < minDistance) {
        minDistance = topDistance;
        normal = Vector2f{0.0f, -1.0f};
    }
    if (bottomDistance < minDistance) {
        normal = Vector2f{0.0f, 1.0f};
    }
    return normal;
}

Vector2f normalFromCircleToRect(const Vector2f& center, const Vector2i& rectMin,
                                const Vector2i& rectMax) {
    float nearestX = center.x;
    if (nearestX < rectMin.x) {
        nearestX = (float)rectMin.x;
    } else if (nearestX > rectMax.x) {
        nearestX = (float)rectMax.x;
    }

    float nearestY = center.y;
    if (nearestY < rectMin.y) {
        nearestY = (float)rectMin.y;
    } else if (nearestY > rectMax.y) {
        nearestY = (float)rectMax.y;
    }

    Vector2f delta = center - Vector2f{nearestX, nearestY};
    float lengthSq = delta.x * delta.x + delta.y * delta.y;
    if (lengthSq > 0.0f) {
        float length = sqrtf(lengthSq);
        return delta / length;
    }

    return normalFromPointToRect(center, rectMin, rectMax);
}
}  // namespace

bool circleRectHit(const Vector2i& center, int radius, const Vector2i& rectMin,
                   const Vector2i& rectMax) {
    int nearestX = center.x;
    if (nearestX < rectMin.x) {
        nearestX = rectMin.x;
    } else if (nearestX > rectMax.x) {
        nearestX = rectMax.x;
    }

    int nearestY = center.y;
    if (nearestY < rectMin.y) {
        nearestY = rectMin.y;
    } else if (nearestY > rectMax.y) {
        nearestY = rectMax.y;
    }

    int dx = center.x - nearestX;
    int dy = center.y - nearestY;
    return dx * dx + dy * dy <= radius * radius;
}

bool aabbHit(const Vector2i& aMin, const Vector2i& aMax, const Vector2i& bMin,
             const Vector2i& bMax) {
    if (aMax.x < bMin.x || bMax.x < aMin.x) {
        return false;
    }
    if (aMax.y < bMin.y || bMax.y < aMin.y) {
        return false;
    }
    return true;
}

bool circleCircleHit(const Vector2i& aCenter, int aRadius, const Vector2i& bCenter, int bRadius) {
    Vector2i delta = aCenter - bCenter;
    int radiusSum = aRadius + bRadius;
    return delta.x * delta.x + delta.y * delta.y <= radiusSum * radiusSum;
}

bool pointInRect(const Vector2i& point, const Vector2i& rectMin, const Vector2i& rectMax) {
    return point.x >= rectMin.x && point.x <= rectMax.x && point.y >= rectMin.y &&
           point.y <= rectMax.y;
}

bool raycastToRect(const Vector2i& origin, const Vector2i& delta, const Vector2i& rectMin,
                   const Vector2i& rectMax, float* tHit) {
    float t0 = 0.0f;
    float t1 = 1.0f;

    auto update = [&](int p, int q) -> bool {
        if (p == 0) {
            return q >= 0;
        }

        float t = (float)q / (float)p;
        if (p < 0) {
            if (t > t1) {
                return false;
            }
            if (t > t0) {
                t0 = t;
            }
        } else {
            if (t < t0) {
                return false;
            }
            if (t < t1) {
                t1 = t;
            }
        }
        return true;
    };

    if (!update(-delta.x, origin.x - rectMin.x)) {
        return false;
    }
    if (!update(delta.x, rectMax.x - origin.x)) {
        return false;
    }
    if (!update(-delta.y, origin.y - rectMin.y)) {
        return false;
    }
    if (!update(delta.y, rectMax.y - origin.y)) {
        return false;
    }

    if (tHit) {
        *tHit = t0;
    }
    return true;
}

bool collidablesHit(const ICollidable& first, const ICollidable& second) {
    return collidablesCollision(first, second).isColliding();
}

ColliderCollision collidablesCollision(const ICollidable& first, const ICollidable& second) {
    ColliderCollision collision;
    CollisionShape firstShape = first.getCollisionShape();
    CollisionShape secondShape = second.getCollisionShape();
    Vector2f firstPosition = first.getCollisionPosition();
    Vector2f secondPosition = second.getCollisionPosition();
    Vector2i firstCenter = floorVector(firstPosition);
    Vector2i secondCenter = floorVector(secondPosition);

    if (firstShape.type() == CollisionShapeType::Point &&
        secondShape.type() == CollisionShapeType::Point) {
        if (firstCenter == secondCenter) {
            collision.setColliding(true);
            collision.setPosition(firstCenter);
            collision.setNormal(Vector2f{0.0f, 0.0f});
        }
        return collision;
    }

    if (firstShape.type() == CollisionShapeType::Circle &&
        secondShape.type() == CollisionShapeType::Circle) {
        if (!circleCircleHit(firstCenter, firstShape.radius(), secondCenter, secondShape.radius())) {
            return collision;
        }
        Vector2f delta = firstPosition - secondPosition;
        float len = sqrtf(delta.x * delta.x + delta.y * delta.y);
        Vector2f normal = len > 0.0f ? delta / len : Vector2f{1.0f, 0.0f};
        collision.setColliding(true);
        collision.setPosition((firstPosition + secondPosition) / 2.0f);
        collision.setNormal(normal);
        return collision;
    }

    if ((firstShape.type() == CollisionShapeType::Rect ||
         firstShape.type() == CollisionShapeType::Point) &&
        (secondShape.type() == CollisionShapeType::Rect ||
         secondShape.type() == CollisionShapeType::Point)) {
        Vector2i firstMin = rectMinFor(first);
        Vector2i firstMax = rectMaxFor(first);
        Vector2i secondMin = rectMinFor(second);
        Vector2i secondMax = rectMaxFor(second);
        if (!aabbHit(firstMin, firstMax, secondMin, secondMax)) {
            return collision;
        }
        collision.setColliding(true);
        collision.setPosition((firstPosition + secondPosition) / 2.0f);
        collision.setNormal(normalFromRects(first, second));
        return collision;
    }

    if (firstShape.type() == CollisionShapeType::Circle &&
        (secondShape.type() == CollisionShapeType::Rect ||
         secondShape.type() == CollisionShapeType::Point)) {
        Vector2i secondMin = rectMinFor(second);
        Vector2i secondMax = rectMaxFor(second);
        if (!circleRectHit(firstCenter, firstShape.radius(), secondMin, secondMax)) {
            return collision;
        }
        collision.setColliding(true);
        Vector2f normal = normalFromCircleToRect(firstPosition, secondMin, secondMax);
        float radius = (float)firstShape.radius();
        collision.setPosition(firstPosition - (normal * radius));
        collision.setNormal(normal);
        return collision;
    }

    if ((firstShape.type() == CollisionShapeType::Rect ||
         firstShape.type() == CollisionShapeType::Point) &&
        secondShape.type() == CollisionShapeType::Circle) {
        ColliderCollision inverted = collidablesCollision(second, first);
        if (!inverted.isColliding()) {
            return collision;
        }
        collision = inverted;
        collision.setNormal(-inverted.normal());
        return collision;
    }

    return collision;
}
