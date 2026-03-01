#pragma once

#include <Arduino.h>
#include <stdint.h>

#include "InputPin.h"

#ifndef DEBOUNCE_INPUT_MS
#define DEBOUNCE_INPUT_MS 18
#endif

class DebouncedInputPin : public InputPin {
   public:
    DebouncedInputPin() = default;

    explicit DebouncedInputPin(uint8_t pinNumber, bool activeLow = true);

    bool update();
    bool isActive() const override;

   private:
    bool stablePressedFlag = false;
    bool lastRawPressed = false;
    uint32_t lastRawChangeMs = 0;
};
