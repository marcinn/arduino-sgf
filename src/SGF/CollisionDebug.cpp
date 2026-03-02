#include "CollisionDebug.h"

#include "CollisionBinding.h"
#include "CollisionShape.h"

namespace {
Vector2i floorVector(const Vector2f& value) {
    return Vector2i{(int)value.x, (int)value.y};
}
}  // namespace

void CollisionDebug::drawSystem(const CollisionSystem& collisionSystem, IFillRect& fillRect,
                                uint16_t shapeColor565, uint16_t collisionColor565) {
    const CollisionBinding* bindings = collisionSystem.bindings();
    for (size_t i = 0; i < collisionSystem.bindingCount(); ++i) {
        drawBinding(bindings[i], fillRect, shapeColor565, collisionColor565);
    }
}

void CollisionDebug::drawBinding(const CollisionBinding& binding, IFillRect& fillRect,
                                 uint16_t shapeColor565, uint16_t collisionColor565) {
    drawCollidable(binding.first(), fillRect, shapeColor565);
    drawCollidable(binding.second(), fillRect, shapeColor565);
    drawCollision(binding.state().collision(), fillRect, collisionColor565);
}

void CollisionDebug::drawCollidable(const ICollidable& collidable, IFillRect& fillRect,
                                    uint16_t color565) {
    Vector2f center = collidable.getCollisionPosition();
    CollisionShape shape = collidable.getCollisionShape();
    Vector2i centerPixel = floorVector(center);

    if (shape.type() == CollisionShapeType::Point) {
        drawPoint(centerPixel, fillRect, color565);
        return;
    }

    if (shape.type() == CollisionShapeType::Circle) {
        int radius = shape.radius();
        drawRectOutline(centerPixel - Vector2i{radius, radius}, centerPixel + Vector2i{radius, radius},
                        fillRect, color565);
        drawPoint(centerPixel, fillRect, color565);
        return;
    }

    Vector2i size = shape.size();
    Vector2i halfSize = size / 2;
    drawRectOutline(centerPixel - halfSize, centerPixel - halfSize + size - Vector2i{1, 1},
                    fillRect, color565);
    drawPoint(centerPixel, fillRect, color565);
}

void CollisionDebug::drawCollision(const ColliderCollision& collision, IFillRect& fillRect,
                                   uint16_t color565) {
    if (!collision.isColliding()) {
        return;
    }

    Vector2i position = floorVector(collision.position());
    drawPoint(position, fillRect, color565);
    drawNormal(position, collision.normal(), fillRect, color565);
}

void CollisionDebug::drawPoint(const Vector2i& position, IFillRect& fillRect, uint16_t color565) {
    fillRect.fillRect565(position.x - 1, position.y - 1, 3, 3, color565);
}

void CollisionDebug::drawRectOutline(const Vector2i& minPosition, const Vector2i& maxPosition,
                                     IFillRect& fillRect, uint16_t color565) {
    int width = maxPosition.x - minPosition.x + 1;
    int height = maxPosition.y - minPosition.y + 1;
    if (width <= 0 || height <= 0) {
        return;
    }

    fillRect.fillRect565(minPosition.x, minPosition.y, width, 1, color565);
    fillRect.fillRect565(minPosition.x, maxPosition.y, width, 1, color565);
    fillRect.fillRect565(minPosition.x, minPosition.y, 1, height, color565);
    fillRect.fillRect565(maxPosition.x, minPosition.y, 1, height, color565);
}

void CollisionDebug::drawNormal(const Vector2i& position, const Vector2f& normal,
                                IFillRect& fillRect, uint16_t color565) {
    Vector2i end = position + Vector2i{(int)(normal.x * 6.0f), (int)(normal.y * 6.0f)};
    int dx = end.x - position.x;
    int dy = end.y - position.y;
    int steps = dx < 0 ? -dx : dx;
    int absDy = dy < 0 ? -dy : dy;
    if (absDy > steps) {
        steps = absDy;
    }
    if (steps <= 0) {
        return;
    }

    for (int step = 0; step <= steps; step++) {
        int x = position.x + (dx * step) / steps;
        int y = position.y + (dy * step) / steps;
        fillRect.fillRect565(x, y, 1, 1, color565);
    }
}
