#pragma once

#include "ActionState.h"

class PressReleaseAction {
   public:
    void reset();
    bool update(const ActionState& action);

   private:
    bool armed = false;
};
