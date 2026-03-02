#pragma once

#include "CollisionShape.h"
#include "ICollidable.h"
#include "Vector2.h"

class CharacterBody : public ICollidable {
   public:
    using Position = Vector2i;

    virtual ~CharacterBody();

    Vector2i getPosition() const;
    void setPosition(const Position& pos);
    void setPosition(int newX, int newY);

    Vector2f getCollisionPosition() const override;
    CollisionShape getCollisionShape() const override;
    void setCollisionShape(const CollisionShape& collisionShapeValue);

   private:
    int posX = 0;
    int posY = 0;
    CollisionShape collisionShape{};
};
