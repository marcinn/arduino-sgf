#include "SceneSwitcher.h"

#include "Scene.h"

void SceneSwitcher::setInitial(Scene& scene) {
  currentScene = &scene;
  currentScene->onEnter();
}

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

const Scene* SceneSwitcher::current() const {
  return currentScene;
}

bool SceneSwitcher::hasCurrent() const {
  return currentScene != nullptr;
}
