#include "DigitalAction.h"

void DigitalAction::update(bool isPressedNow) {
  justPressedFlag = isPressedNow && !pressedFlag;
  justReleasedFlag = !isPressedNow && pressedFlag;
  pressedFlag = isPressedNow;
}

void DigitalAction::reset(bool isPressedNow) {
  pressedFlag = isPressedNow;
  justPressedFlag = false;
  justReleasedFlag = false;
}

bool DigitalAction::pressed() const { return pressedFlag; }
bool DigitalAction::justPressed() const { return justPressedFlag; }
bool DigitalAction::justReleased() const { return justReleasedFlag; }
