#include "SceneSwitcher.h"

#include "Scene.h"

void SceneSwitcher::switchTo(Scene& scene) {
    if (currentScene == &scene) {
        return;
    }
    if (currentScene) {
        currentScene->onExit();
    }
    currentScene = &scene;
    currentScene->onEnter();
}

void SceneSwitcher::onAction(ActionState& action) {
    if (currentScene) {
        currentScene->onAction(action);
    }
}

void SceneSwitcher::onInput(const InputEvent& event) {
    if (currentScene) {
        currentScene->onInput(event);
    }
}

void SceneSwitcher::onPhysics(float delta) {
    if (currentScene) {
        currentScene->onPhysics(delta);
    }
}

void SceneSwitcher::onProcess(float delta) {
    if (currentScene) {
        currentScene->onProcess(delta);
    }
}

const Scene* SceneSwitcher::current() const { return currentScene; }

bool SceneSwitcher::hasCurrent() const { return currentScene != nullptr; }
