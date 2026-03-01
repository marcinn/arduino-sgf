#include "BacklightFade.h"

void BacklightFade::stop() { active = false; }

void BacklightFade::start(uint8_t initialLevel, uint8_t finalLevel, uint32_t nowMs,
                          uint32_t fadeDurationMs) {
    active = true;
    startLevel = initialLevel;
    targetLevel = finalLevel;
    startMs = nowMs;
    durationMs = fadeDurationMs;
}

uint8_t BacklightFade::levelAt(uint32_t nowMs) const {
    if (!active || durationMs == 0u) {
        return targetLevel;
    }

    uint32_t elapsed = nowMs - startMs;
    if (elapsed >= durationMs) {
        return targetLevel;
    }

    int32_t delta = (int32_t)targetLevel - (int32_t)startLevel;
    return (uint8_t)((int32_t)startLevel + (delta * (int32_t)elapsed) / (int32_t)durationMs);
}

bool BacklightFade::isComplete(uint32_t nowMs) const {
    return !active || durationMs == 0u || (nowMs - startMs) >= durationMs;
}
