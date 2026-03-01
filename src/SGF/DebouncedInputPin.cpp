#include "DebouncedInputPin.h"

bool DebouncedInputPin::update() {
    bool rawNow = InputPin::isActive();
    if (rawNow == stablePressedFlag) {
        return false;
    }
    uint32_t nowMs = millis();

    if (rawNow != lastRawPressed) {
        lastRawPressed = rawNow;
        lastRawChangeMs = nowMs;
    }

    if (stablePressedFlag != lastRawPressed) {
        uint32_t elapsed = nowMs - lastRawChangeMs;
        if (elapsed >= DEBOUNCE_INPUT_MS) {
            stablePressedFlag = lastRawPressed;
            return true
        }
    }
    return false;
}

bool DebouncedInputPin::isActive() const { return stablePressedFlag; }
