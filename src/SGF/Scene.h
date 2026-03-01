#pragma once

class Scene {
   public:
    virtual ~Scene() = default;

    virtual void onEnter() {}
    virtual void onExit() {}
    virtual void onPhysics(float delta) = 0;
    virtual void onProcess(float delta) = 0;
};

#include "SceneSwitcher.h"
