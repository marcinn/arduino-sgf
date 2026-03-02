#pragma once

#include "ColliderCollision.h"
#include "ICollidable.h"
#include "IFillRect.h"
#include "Vector2.h"

#include <stdint.h>

class CollisionDebug {
   public:
    static void drawCollidable(const ICollidable& collidable, IFillRect& fillRect,
                               uint16_t color565);
    static void drawCollision(const ColliderCollision& collision, IFillRect& fillRect,
                              uint16_t color565);
    static void drawTileCollision(const ColliderCollision& collision, const Vector2i& tileSize,
                                  const Vector2f& minBounds, IFillRect& fillRect,
                                  uint16_t color565);

   private:
    static void drawPoint(const Vector2i& position, IFillRect& fillRect, uint16_t color565);
    static void drawRectOutline(const Vector2i& minPosition, const Vector2i& maxPosition,
                                IFillRect& fillRect, uint16_t color565);
    static void drawNormal(const Vector2i& position, const Vector2f& normal, IFillRect& fillRect,
                           uint16_t color565);
};
