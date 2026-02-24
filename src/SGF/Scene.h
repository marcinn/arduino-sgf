#pragma once

class Scene {
public:
  virtual ~Scene() = default;

  virtual void onEnter() {}
  virtual void onExit() {}
  virtual void onPhysics(float delta) = 0;
  virtual void onProcess(float delta) = 0;
};

class SceneSwitcher {
public:
  void setInitial(Scene& scene) {
    currentScene = &scene;
    currentScene->onEnter();
  }

  void switchTo(Scene& scene) {
    if (currentScene == &scene) {
      return;
    }
    if (currentScene) {
      currentScene->onExit();
    }
    currentScene = &scene;
    currentScene->onEnter();
  }

  void onPhysics(float delta) {
    if (currentScene) {
      currentScene->onPhysics(delta);
    }
  }

  void onProcess(float delta) {
    if (currentScene) {
      currentScene->onProcess(delta);
    }
  }

  const Scene* current() const { return currentScene; }
  bool hasCurrent() const { return currentScene != nullptr; }

private:
  Scene* currentScene = nullptr;
};
