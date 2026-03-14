#include "ActionState.h"

void ActionState::update(bool activeNow) {
  previousPressed = currentPressed;
  currentPressed = activeNow;
}

void ActionState::reset() {
  previousPressed = currentPressed = false;
}

void ActionState::sync(bool activeNow) {
  previousPressed = activeNow;
  currentPressed = activeNow;
}

bool ActionState::isPressed() const {
  return currentPressed;
}

bool ActionState::isJustPressed() const {
  return currentPressed && !previousPressed;
}

bool ActionState::isJustReleased() const {
  return !currentPressed && previousPressed;
}
