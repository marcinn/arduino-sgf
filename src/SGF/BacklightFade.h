#pragma once

#include <stdint.h>

struct BacklightFade {
  bool active = false;
  uint8_t startLevel = 255u;
  uint8_t targetLevel = 255u;
  uint32_t startMs = 0;
  uint32_t durationMs = 0;

  void stop();
  void start(uint8_t initialLevel, uint8_t finalLevel, uint32_t nowMs, uint32_t fadeDurationMs);
  uint8_t levelAt(uint32_t nowMs) const;
  bool isComplete(uint32_t nowMs) const;
};
