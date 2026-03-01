#include "DebouncedInputPin.h"

DebouncedInputPin::DebouncedInputPin(uint8_t pinNumber,
                                     bool activeLow,
                                     uint16_t debounceMs)
  : InputPin(pinNumber, activeLow), debounceMsValue(debounceMs) {}

void DebouncedInputPin::setDebounceMs(uint16_t debounceMs) {
  debounceMsValue = debounceMs;
}

uint16_t DebouncedInputPin::debounceMs() const {
  return debounceMsValue;
}

void DebouncedInputPin::reset(bool pressedNow) {
  stablePressedFlag = pressedNow;
  lastRawPressed = pressedNow;
  lastRawChangeMs = millis();
}

void DebouncedInputPin::resetFromPin() {
  reset(InputPin::pressed());
}

bool DebouncedInputPin::update() {
  bool rawNow = InputPin::pressed();
  uint32_t nowMs = millis();

  if (rawNow != lastRawPressed) {
    lastRawPressed = rawNow;
    lastRawChangeMs = nowMs;
  }

  if (stablePressedFlag != lastRawPressed) {
    uint32_t elapsed = nowMs - lastRawChangeMs;
    if (elapsed >= debounceMsValue) {
      stablePressedFlag = lastRawPressed;
    }
  }

  return stablePressedFlag;
}

bool DebouncedInputPin::pressed() const {
  return stablePressedFlag;
}
