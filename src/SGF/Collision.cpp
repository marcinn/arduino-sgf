#include "Collision.h"

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
