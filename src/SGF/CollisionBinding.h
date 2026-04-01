#pragma once

#include "ICollidable.h"
#include "CollisionState.h"

class CollisionBinding {
   public:
    CollisionBinding(ICollidable& firstCollidable, ICollidable& secondCollidable,
                     CollisionState& collisionState);

    ICollidable& first() const;
    ICollidable& second() const;
    CollisionState& state() const;

   private:
    ICollidable* firstCollidable = nullptr;
    ICollidable* secondCollidable = nullptr;
    CollisionState* bindingState = nullptr;
};
