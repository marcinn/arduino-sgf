#pragma once

#include "DigitalAction.h"

class PressReleaseAction {
public:
  void reset();
  bool update(const DigitalAction& action);

private:
  bool armed = false;
};
