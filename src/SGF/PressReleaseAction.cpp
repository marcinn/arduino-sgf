#include "PressReleaseAction.h"

void PressReleaseAction::reset() {
  armed = false;
}

bool PressReleaseAction::update(const ActionState& action) {
  if (action.isJustPressed()) {
    armed = true;
  }
  if (armed && action.isJustReleased()) {
    armed = false;
    return true;
  }
  return false;
}
