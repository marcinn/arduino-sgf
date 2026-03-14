#pragma once

#include <stddef.h>
#include <stdint.h>

#include "Vector2.h"

class IRenderTarget {
   public:
    virtual ~IRenderTarget() = default;
    virtual Vector2i size() const = 0;
    virtual void blit565(int x0, int y0, int w, int h, const uint16_t* pix) = 0;
    virtual bool supportsBlit565Stream() const { return false; }
    virtual void beginBlit565Stream(int x0, int y0, int w, int h) {
        (void)x0;
        (void)y0;
        (void)w;
        (void)h;
    }
    virtual void writeBlit565StreamChunk(const uint16_t* pix, size_t count) {
        (void)pix;
        (void)count;
    }
    virtual bool supportsPreSwappedBlit565Stream() const { return false; }
    virtual void writePreSwappedBlit565StreamChunk(const uint16_t* pix, size_t count) {
        writeBlit565StreamChunk(pix, count);
    }
    virtual bool supportsQueuedPreSwappedBlit565Stream() const { return false; }
    virtual void waitQueuedPreSwappedBlit565StreamSlot() {}
    virtual void queuePreSwappedBlit565StreamChunk(const uint16_t* pix, size_t count) {
        writePreSwappedBlit565StreamChunk(pix, count);
    }
    virtual void endBlit565Stream() {}
    virtual void tickEffects() {}
    virtual bool supportsHardwareScroll() const { return false; }
    virtual void setScrollArea(uint16_t fixedStart, uint16_t scrollSpan, uint16_t fixedEnd) {
        (void)fixedStart;
        (void)scrollSpan;
        (void)fixedEnd;
    }
    virtual void scrollTo(uint16_t offset) { (void)offset; }
    virtual bool scrollAxisInverted() const { return false; }
};
