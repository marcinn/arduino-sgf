#include <Arduino.h>

#include <Arduino.h>

#include "Game.h"

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
  float dtSec = tickSeconds(micros());
  onPhysics(dtSec);
  onProcess(dtSec);
}

void Game::resetClock() {
  clock.lastUs = micros();
}

float Game::tickSeconds(uint32_t nowUs) {
  if (clock.lastUs == 0) {
    clock.lastUs = nowUs;
    return (float)clock.defaultStepUs / 1000000.0f;
  }

  uint32_t dtUs = nowUs - clock.lastUs;
  clock.lastUs = nowUs;

  if (clock.maxStepUs != 0 && dtUs > clock.maxStepUs) dtUs = clock.maxStepUs;
  return (float)dtUs / 1000000.0f;
}
