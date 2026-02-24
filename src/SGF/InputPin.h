#pragma once

#include <Arduino.h>
#include <stdint.h>

class InputPin {
public:
  InputPin() = default;

  explicit InputPin(uint8_t pinNumber, bool activeLow = true)
    : pinNumber(pinNumber), activeLow(activeLow) {}

  void attach(uint8_t newPinNumber, bool isActiveLow = true) {
    pinNumber = newPinNumber;
    activeLow = isActiveLow;
  }

  void begin(uint8_t mode) const {
    pinMode(pinNumber, mode);
  }

  bool rawPressed() const {
    int level = digitalRead(pinNumber);
    return activeLow ? (level == LOW) : (level == HIGH);
  }

  uint8_t pin() const { return pinNumber; }

private:
  uint8_t pinNumber = 0;
  bool activeLow = true;
};

class DebouncedInputPin {
public:
  DebouncedInputPin() = default;

  explicit DebouncedInputPin(uint8_t pinNumber,
                             bool activeLow = true,
                             uint16_t debounceMs = 18)
    : input(pinNumber, activeLow), debounceMsValue(debounceMs) {}

  void attach(uint8_t pinNumber, bool activeLow = true) {
    input.attach(pinNumber, activeLow);
  }

  void begin(uint8_t mode) const {
    input.begin(mode);
  }

  void setDebounceMs(uint16_t debounceMs) {
    debounceMsValue = debounceMs;
  }

  uint16_t debounceMs() const { return debounceMsValue; }

  void reset(bool pressedNow = false) {
    stablePressedFlag = pressedNow;
    lastRawPressed = pressedNow;
    lastRawChangeMs = millis();
  }

  void resetFromPin() {
    reset(input.rawPressed());
  }

  bool update() {
    bool rawNow = input.rawPressed();
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

  bool pressed() const { return stablePressedFlag; }
  bool rawPressed() const { return input.rawPressed(); }
  uint8_t pin() const { return input.pin(); }

private:
  InputPin input;
  uint16_t debounceMsValue = 18;
  bool stablePressedFlag = false;
  bool lastRawPressed = false;
  uint32_t lastRawChangeMs = 0;
};
