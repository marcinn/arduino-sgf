#include "InputPin.h"

#include <Arduino.h>

InputPin::InputPin(uint8_t pinNumber, bool activeLow) { attach(pinNumber, activeLow); }

void InputPin::attach(uint8_t pinNumber, bool activeLow) {
    this->pinNumber = pinNumber;
    this->activeLow = activeLow;
    pinMode(pinNumber, activeLow ? INPUT_PULLUP : INPUT);
    update();
}

void InputPin::update() {
    int level = digitalRead(pinNumber);
    pressed = activeLow ? (level == LOW) : (level == HIGH);
}

bool InputPin::isActive() const {
    return pressed;
}

uint8_t InputPin::pin() const { return pinNumber; }
