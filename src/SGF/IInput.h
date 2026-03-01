#pragma once

#include <stdint.h>

class IInput {
   public:
    virtual ~IInput() = default;
    virtual bool isActive() const = 0;
};
