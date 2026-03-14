#pragma once

#include "IFillRect.h"

class BufferFillRect : public IFillRect {
   public:
    BufferFillRect(int regionX0, int regionY0, int regionW, int regionH, uint16_t* buf);

    void fillRect565(int x0, int y0, int w, int h, uint16_t color565) override;

   private:
    int regionX0;
    int regionY0;
    int regionW;
    int regionH;
    uint16_t* buf;
};
