#include "BufferFillRect.h"

#include <Arduino.h>

BufferFillRect::BufferFillRect(int regionX0,
                               int regionY0,
                               int regionW,
                               int regionH,
                               uint16_t* buf)
    : regionX0(regionX0), regionY0(regionY0), regionW(regionW), regionH(regionH), buf(buf) {}

void BufferFillRect::fillRect565(int x0, int y0, int w, int h, uint16_t color565) {
    if (buf == nullptr || w <= 0 || h <= 0) {
        return;
    }

    int x1 = x0 + w;
    int y1 = y0 + h;
    int clipX0 = max(regionX0, x0);
    int clipY0 = max(regionY0, y0);
    int clipX1 = min(regionX0 + regionW, x1);
    int clipY1 = min(regionY0 + regionH, y1);
    if (clipX0 >= clipX1 || clipY0 >= clipY1) {
        return;
    }

    for (int yy = clipY0; yy < clipY1; ++yy) {
        uint16_t* row = buf + (yy - regionY0) * regionW;
        for (int xx = clipX0; xx < clipX1; ++xx) {
            row[xx - regionX0] = color565;
        }
    }
}
