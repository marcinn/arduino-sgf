#pragma once

#include <stdint.h>

class ActionState {
public:
  void update(bool activeNow);
  void reset();
  void sync(bool activeNow);

  bool isPressed() const;
  bool isJustPressed() const;
  bool isJustReleased() const;

private:
  bool currentPressed = false;
  bool previousPressed = false;
};
