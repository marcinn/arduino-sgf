#pragma once

#include <stdint.h>

class DigitalAction {
public:
  void update(bool isPressedNow) {
    justPressedFlag = isPressedNow && !pressedFlag;
    justReleasedFlag = !isPressedNow && pressedFlag;
    pressedFlag = isPressedNow;
  }

  void reset(bool isPressedNow = false) {
    pressedFlag = isPressedNow;
    justPressedFlag = false;
    justReleasedFlag = false;
  }

  bool pressed() const { return pressedFlag; }
  bool justPressed() const { return justPressedFlag; }
  bool justReleased() const { return justReleasedFlag; }

private:
  bool pressedFlag = false;
  bool justPressedFlag = false;
  bool justReleasedFlag = false;
};

class PressReleaseAction {
public:
  void reset() { armed = false; }

  bool update(const DigitalAction& action) {
    if (action.justPressed()) {
      armed = true;
    }
    if (armed && action.justReleased()) {
      armed = false;
      return true;
    }
    return false;
  }

private:
  bool armed = false;
};
