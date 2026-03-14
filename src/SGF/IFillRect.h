#pragma once

#include <stdint.h>

class IFillRect {
   public:
    virtual ~IFillRect() = default;
    virtual void fillRect565(int x0, int y0, int w, int h, uint16_t color565) = 0;
};
