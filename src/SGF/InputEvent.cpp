#include "InputEvent.h"

#include "ActionState.h"

bool InputEvent::isActionPressed(const ActionState& action) const {
    return this->action == &action && pressed;
}

bool InputEvent::isActionJustPressed(const ActionState& action) const {
    return this->action == &action && justPressed;
}

bool InputEvent::isActionJustReleased(const ActionState& action) const {
    return this->action == &action && justReleased;
}
