#pragma once

#include <Arduino.h>
#include <stdint.h>

#include "InputPin.h"

class DebouncedInputPin : public InputPin {
public:
  DebouncedInputPin() = default;

  explicit DebouncedInputPin(uint8_t pinNumber,
                             bool activeLow = true,
                             uint16_t debounceMs = 18);

  void setDebounceMs(uint16_t debounceMs);
  uint16_t debounceMs() const;

  void reset(bool pressedNow = false);
  void resetFromPin();

  bool update();
  bool pressed() const override;

private:
  uint16_t debounceMsValue = 18;
  bool stablePressedFlag = false;
  bool lastRawPressed = false;
  uint32_t lastRawChangeMs = 0;
};
