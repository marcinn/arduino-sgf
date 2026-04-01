#pragma once

#include "ActionState.h"
#include "IInput.h"

struct ActionBinding {
    IInput& input;
    ActionState& state;
};
