#include "CollisionBinding.h"

CollisionBinding::CollisionBinding(ICollidable& firstCollidable, ICollidable& secondCollidable,
                                   CollisionState& collisionState)
    : firstCollidable(&firstCollidable), secondCollidable(&secondCollidable),
      bindingState(&collisionState) {}

ICollidable& CollisionBinding::first() const { return *firstCollidable; }

ICollidable& CollisionBinding::second() const { return *secondCollidable; }

CollisionState& CollisionBinding::state() const { return *bindingState; }
