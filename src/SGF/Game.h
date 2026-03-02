#pragma once

#include <stddef.h>
#include <stdint.h>

#include "ActionState.h"
#include "ActionBinding.h"
#include "InputEvent.h"
#include "IRenderer.h"
#include "SceneSwitcher.h"

class Scene;

class Game {
    public:
    Game(uint32_t defaultStepUs, uint32_t maxStepUs);
    virtual ~Game() = default;

    void start();
    void loop();

    void switchScene(Scene& scene);
    const Scene* currentScene() const;
    bool hasCurrentScene() const;
    void resetActions();

    protected:
    virtual void onSetup() = 0;
    virtual void onPhysics(float delta) { (void)delta; }
    virtual void onProcess(float delta) { (void)delta; }
    virtual void onAction(ActionState& action) {}
    virtual void onInput(const InputEvent& event) {}

    void configureActions(const ActionBinding* bindings, size_t count);
    void attachRenderer(IRenderer& renderer) { this->renderer = &renderer; }
    const InputEvent& inputEvent() const { return currentInputEvent; }

    private:
    struct FrameClock {
        uint32_t lastUs;
        uint32_t defaultStepUs;
        uint32_t maxStepUs;
    };

    FrameClock clock;
    const ActionBinding* actionBindings = nullptr;
    size_t actionBindingCount = 0;
    InputEvent currentInputEvent;
    IRenderer* renderer = nullptr;
    SceneSwitcher sceneSwitcher;

    void resetClock();
    void updateActionStates();
    float tickSeconds(uint32_t nowUs);
};
