#include "DebouncedInputPin.h"

void DebouncedInputPin::update() {
    bool rawNow = InputPin::isActive();
    if (rawNow == stablePressedFlag) {
        return;
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
            return;
        }
    }
}

bool DebouncedInputPin::isActive() const { return stablePressedFlag; }
