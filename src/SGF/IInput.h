#pragma once

#include <stdint.h>

class IInput {
   public:
    virtual ~IInput() = default;
    virtual void update() = 0;
    virtual bool isActive() const = 0;
};
