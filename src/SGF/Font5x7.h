#pragma once

#include <Arduino.h>

#include "IFont.h"

class Font5x7 : public IFont {
   public:
    int glyphWidth() const override;
    int glyphHeight() const override;
    int glyphAdvance() const override;
    uint8_t glyphRowBits(char ch, int row) const override;
};

extern const Font5x7 FONT_5X7;
