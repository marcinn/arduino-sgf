#pragma once

#include <stdint.h>

class IFont {
   public:
    virtual ~IFont();

    virtual int glyphWidth() const = 0;
    virtual int glyphHeight() const = 0;
    virtual int glyphAdvance() const = 0;
    virtual uint8_t glyphRowBits(char ch, int row) const = 0;
};
