#pragma once

#include <Arduino.h>
#include <stdint.h>

class InputPin {
public:
  InputPin() = default;

  InputPin(uint8_t pinNumber, bool activeLow = true);
  void attach(uint8_t pinNumber, bool activeLow = true);
  virtual bool pressed() const;
  uint8_t pin() const;

private:
  uint8_t pinNumber = 0;
  bool activeLow = true;
};
