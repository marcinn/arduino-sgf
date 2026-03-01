#include "InputPin.h"

#include <Arduino.h>

InputPin::InputPin(uint8_t pinNumber, bool activeLow) { attach(pinNumber, activeLow); }

void InputPin::attach(uint8_t pinNumber, bool activeLow) {
    this->pinNumber = pinNumber;
    this->activeLow = activeLow;
    pinMode(pinNumber, activeLow ? INPUT_PULLUP : INPUT);
}

bool InputPin::isActive() const {
    int level = digitalRead(pinNumber);
    return activeLow ? (level == LOW) : (level == HIGH);
}

uint8_t InputPin::pin() const { return pinNumber; }
