#include "DebouncedInputPin.h"

DebouncedInputPin::DebouncedInputPin(uint8_t pinNumber, bool activeLow) : InputPin(pinNumber, activeLow) {
    stablePressedFlag = InputPin::isActive();
    lastRawPressed = stablePressedFlag;
    lastRawChangeMs = millis();
}

void DebouncedInputPin::update() {
    InputPin::update();
    bool rawNow = InputPin::isActive();
    if (rawNow == stablePressedFlag) {
        lastRawPressed = rawNow;
        lastRawChangeMs = millis();
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
