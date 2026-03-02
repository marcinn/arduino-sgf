#include "Game.h"

#include <Arduino.h>

#include "Scene.h"
#include "SceneSwitcher.h"

namespace {
uint32_t frameIntervalUs(uint32_t fps) {
    if (fps == 0) {
        return 0;
    }
    return 1000000u / fps;
}
}  // namespace

Game::Game(uint32_t defaultStepUs, uint32_t maxStepUs) {
    physicsClock.lastUs = 0;
    physicsClock.defaultStepUs = defaultStepUs;
    physicsClock.maxStepUs = maxStepUs;
}

void Game::start() {
    physicsClock.lastUs = 0;
    lastProcessUs = 0;
    lastRenderUs = 0;
    onSetup();
    resetClock();
    resetActions();
}

void Game::loop() {
    uint32_t nowUs = micros();
    float processDelta = processSeconds(nowUs);
    onProcess(processDelta);
    sceneSwitcher.onProcess(processDelta);

    uint32_t physicsInterval = frameIntervalUs(PHYSICS_TARGET_FPS);
    if (physicsInterval == 0 || nowUs - physicsClock.lastUs >= physicsInterval) {
        float delta = tickSeconds(nowUs);
        updateActionStates();
        onPhysics(delta);
        sceneSwitcher.onPhysics(delta);
    }

    uint32_t renderInterval = frameIntervalUs(RENDER_TARGET_FPS);
    if (renderer && (renderInterval == 0 || nowUs - lastRenderUs >= renderInterval)) {
        lastRenderUs = nowUs;
        renderer->render();
    }
}

void Game::resetClock() {
    uint32_t nowUs = micros();
    physicsClock.lastUs = nowUs;
    lastProcessUs = nowUs;
    lastRenderUs = nowUs;
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
    if (physicsClock.lastUs == 0) {
        physicsClock.lastUs = nowUs;
        return (float)physicsClock.defaultStepUs / 1000000.0f;
    }

    uint32_t dtUs = nowUs - physicsClock.lastUs;
    physicsClock.lastUs = nowUs;

    if (physicsClock.maxStepUs != 0 && dtUs > physicsClock.maxStepUs) {
        dtUs = physicsClock.maxStepUs;
    }
    return (float)dtUs / 1000000.0f;
}

float Game::processSeconds(uint32_t nowUs) {
    if (lastProcessUs == 0) {
        lastProcessUs = nowUs;
        return (float)physicsClock.defaultStepUs / 1000000.0f;
    }

    uint32_t dtUs = nowUs - lastProcessUs;
    lastProcessUs = nowUs;

    if (physicsClock.maxStepUs != 0 && dtUs > physicsClock.maxStepUs) {
        dtUs = physicsClock.maxStepUs;
    }
    return (float)dtUs / 1000000.0f;
}
