#include "Game.h"

#include <Arduino.h>

#include "Scene.h"
#include "SceneSwitcher.h"

Game::Game(uint32_t defaultStepUs, uint32_t maxStepUs) {
    clock.lastUs = 0;
    clock.defaultStepUs = defaultStepUs;
    clock.maxStepUs = maxStepUs;
}

void Game::start() {
    clock.lastUs = 0;
    onSetup();
    resetClock();
}

void Game::loop() {
    updateActionStates();
    float delta = tickSeconds(micros());
    onPhysics(delta);
    sceneSwitcher.onPhysics(delta);
    onProcess(delta);
    sceneSwitcher.onProcess(delta);
}

void Game::resetClock() {
    clock.lastUs = micros();
}

void Game::switchScene(Scene& scene) {
    sceneSwitcher.switchTo(scene);
    resetClock();
}

const Scene* Game::currentScene() const {
    return sceneSwitcher.current();
}

bool Game::hasCurrentScene() const {
    return sceneSwitcher.hasCurrent();
}

void Game::resetActions() {
    for (size_t i = 0; i < actionBindingCount; ++i) {
        actionBindings[i].input.update();
        actionBindings[i].state.sync(actionBindings[i].input.isActive());
    }
}

void Game::configureActions(const ActionBinding* bindings, size_t count) {
    actionBindings = bindings;
    actionBindingCount = count;
    for (size_t i = 0; i < actionBindingCount; ++i) {
        actionBindings[i].state.reset();
    }
}

void Game::updateActionStates() {
    for (size_t i = 0; i < actionBindingCount; ++i) {
        actionBindings[i].input.update();
        bool value = actionBindings[i].input.isActive();
        ActionState& actionState = actionBindings[i].state;
        actionState.update(value);
        if (!actionState.isPressed() &&
            !actionState.isJustPressed() &&
            !actionState.isJustReleased()) {
            continue;
        }

        currentInputEvent.action = &actionState;
        currentInputEvent.pressed = actionState.isPressed();
        currentInputEvent.justPressed = actionState.isJustPressed();
        currentInputEvent.justReleased = actionState.isJustReleased();
        onAction(actionState);
        sceneSwitcher.onAction(actionState);
        onInput(currentInputEvent);
        sceneSwitcher.onInput(currentInputEvent);
    }
}

float Game::tickSeconds(uint32_t nowUs) {
    if (clock.lastUs == 0) {
        clock.lastUs = nowUs;
        return (float)clock.defaultStepUs / 1000000.0f;
    }

    uint32_t dtUs = nowUs - clock.lastUs;
    clock.lastUs = nowUs;

    if (clock.maxStepUs != 0 && dtUs > clock.maxStepUs) {
        dtUs = clock.maxStepUs;
    }
    return (float)dtUs / 1000000.0f;
}
