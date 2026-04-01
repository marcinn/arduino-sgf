#pragma once

#include "CollisionSystem.h"
#include "IFillRect.h"

#include <stdint.h>

class CollisionDebug {
   public:
    static void drawSystem(const CollisionSystem& collisionSystem, IFillRect& fillRect,
                           uint16_t shapeColor565, uint16_t collisionColor565);

   private:
    static void drawBinding(const CollisionBinding& binding, IFillRect& fillRect,
                            uint16_t shapeColor565, uint16_t collisionColor565);
    static void drawCollidable(const ICollidable& collidable, IFillRect& fillRect,
                               uint16_t color565);
    static void drawCollision(const ColliderCollision& collision, IFillRect& fillRect,
                              uint16_t color565);
    static void drawPoint(const Vector2i& position, IFillRect& fillRect, uint16_t color565);
    static void drawRectOutline(const Vector2i& minPosition, const Vector2i& maxPosition,
                                IFillRect& fillRect, uint16_t color565);
    static void drawNormal(const Vector2i& position, const Vector2f& normal, IFillRect& fillRect,
                           uint16_t color565);
};
