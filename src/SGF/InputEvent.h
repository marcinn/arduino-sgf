#pragma once

class ActionState;
class Game;

class InputEvent {
    public:
    bool isActionPressed(const ActionState& action) const;
    bool isActionJustPressed(const ActionState& action) const;
    bool isActionJustReleased(const ActionState& action) const;

    private:
    friend class Game;

    const ActionState* action = nullptr;
    bool pressed = false;
    bool justPressed = false;
    bool justReleased = false;
};
