#pragma once

#include <stdint.h>

class Game {
public:
  Game(uint32_t defaultStepUs, uint32_t maxStepUs);
  virtual ~Game() = default;

  void start();
  void loop();
  void resetClock();

protected:
  virtual void onSetup() = 0;
  virtual void onPhysics(float dtSec) = 0;
  virtual void onProcess(float dtSec) = 0;

private:
  struct FrameClock {
    uint32_t lastUs;
    uint32_t defaultStepUs;
    uint32_t maxStepUs;
  };

  FrameClock clock;

  float tickSeconds(uint32_t nowUs);
};
