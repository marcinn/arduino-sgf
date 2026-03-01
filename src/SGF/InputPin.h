#pragma once

#include <Arduino.h>
#include <stdint.h>

#include "IInput.h"

class InputPin : public IInput {
   public:
    InputPin() = default;

    InputPin(uint8_t pinNumber, bool activeLow = true);
    void attach(uint8_t pinNumber, bool activeLow = true);
    bool isActive() const override;
    uint8_t pin() const;

   private:
    uint8_t pinNumber = 0;
    bool activeLow = true;
};
