#pragma once

class ActionState;
class InputEvent;

class Scene {
   public:
    virtual ~Scene() = default;

    virtual void onEnter() {}
    virtual void onExit() {}
    virtual void onAction(ActionState& action) { (void)action; }
    virtual void onInput(const InputEvent& event) { (void)event; }
    virtual void onPhysics(float delta) = 0;
    virtual void onProcess(float delta) = 0;
};

#include "SceneSwitcher.h"
