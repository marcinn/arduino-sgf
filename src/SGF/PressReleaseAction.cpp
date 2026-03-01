#include "PressReleaseAction.h"

void PressReleaseAction::reset() {
  armed = false;
}

bool PressReleaseAction::update(const DigitalAction& action) {
  if (action.justPressed()) {
    armed = true;
  }
  if (armed && action.justReleased()) {
    armed = false;
    return true;
  }
  return false;
}
