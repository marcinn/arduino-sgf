#pragma once

class Scene;

class SceneSwitcher {
public:
  void setInitial(Scene& scene);
  void switchTo(Scene& scene);
  void onPhysics(float delta);
  void onProcess(float delta);
  const Scene* current() const;
  bool hasCurrent() const;

private:
  Scene* currentScene = nullptr;
};
