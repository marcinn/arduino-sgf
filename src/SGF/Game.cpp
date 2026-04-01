#include "Game.h"

#include <Arduino.h>

#include "Scene.h"
#include "SceneSwitcher.h"
#include "SerialMonitor.h"

#if defined(ENABLE_PROFILER) && ENABLE_PROFILER
#define SGF_ENABLE_PROFILER 1
#else
#define SGF_ENABLE_PROFILER 0
#endif

namespace {
uint32_t frameIntervalUs(uint32_t fps) {
    if (fps == 0) {
        return 0;
    }
    return 1000000u / fps;
}
}  // namespace

Game::Game(uint32_t defaultStepUs, uint32_t maxStepUs)
    : gameProfiler("game", profilerSlots, PROFILER_SLOT_COUNT) {
    physicsClock.lastUs = 0;
    physicsClock.defaultStepUs = defaultStepUs;
    physicsClock.maxStepUs = maxStepUs;

#if SGF_ENABLE_PROFILER
    gameProfiler.setLabel(LoopCountSlot, "loop_calls");
    gameProfiler.setLabel(ProcessCountSlot, "process_calls");
    gameProfiler.setLabel(PhysicsCountSlot, "physics_calls");
    gameProfiler.setLabel(RenderCountSlot, "render_calls");
#endif
}

void Game::start() {
    physicsClock.lastUs = 0;
    lastProcessUs = 0;
    lastRenderUs = 0;
#if SGF_ENABLE_PROFILER
    if (serialMonitor) {
        serialMonitor->begin();
    }
#endif
    onSetup();
    resetClock();
    resetActions();
}

void Game::loop() {
    uint32_t nowUs = micros();
#if SGF_ENABLE_PROFILER
    gameProfiler.increment(LoopCountSlot);
#endif

    float processDelta = processSeconds(nowUs);
    onProcess(processDelta);
    sceneSwitcher.onProcess(processDelta);
#if SGF_ENABLE_PROFILER
    gameProfiler.increment(ProcessCountSlot);
#endif

    uint32_t physicsInterval = frameIntervalUs(PHYSICS_TARGET_FPS);
    if (physicsInterval == 0 || nowUs - physicsClock.lastUs >= physicsInterval) {
        float delta = tickSeconds(nowUs);
        updateActionStates();
        onPhysics(delta);
        sceneSwitcher.onPhysics(delta);
#if SGF_ENABLE_PROFILER
        gameProfiler.increment(PhysicsCountSlot);
#endif
    }

    uint32_t renderInterval = frameIntervalUs(RENDER_TARGET_FPS);
    if (renderer && (renderInterval == 0 || nowUs - lastRenderUs >= renderInterval)) {
        lastRenderUs = nowUs;
        renderer->render();
#if SGF_ENABLE_PROFILER
        gameProfiler.increment(RenderCountSlot);
#endif
    }
#if SGF_ENABLE_PROFILER
    if (serialMonitor) {
        serialMonitor->tick();
    }
#endif
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

void Game::attachSerialMonitor(SerialMonitor& serialMonitor) {
#if SGF_ENABLE_PROFILER
    this->serialMonitor = &serialMonitor;
    serialMonitor.attachProfiler(gameProfiler);
#else
    (void)serialMonitor;
#endif
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
