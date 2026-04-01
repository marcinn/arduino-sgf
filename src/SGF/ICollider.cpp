#include "ICollider.h"

ICollider::~ICollider() = default;

void ICollider::setBodies(ICollidable* const* bodies, size_t count) {
    (void)bodies;
    (void)count;
}

void ICollider::resolveConfiguredBodies() const {}
