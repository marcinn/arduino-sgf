#pragma once

class ActionState;
class InputEvent;
class Scene;

class SceneSwitcher {
   public:
    void setInitial(Scene& scene);
    void switchTo(Scene& scene);
    void onAction(ActionState& action);
    void onInput(const InputEvent& event);
    void onPhysics(float delta);
    void onProcess(float delta);
    const Scene* current() const;
    bool hasCurrent() const;

   private:
    Scene* currentScene = nullptr;
};
