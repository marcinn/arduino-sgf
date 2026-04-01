#pragma once

#include <stdint.h>

#include "AudioTypes.h"
#include "INotePlayer.h"

namespace SGFAudio {

struct PatternStep {
  float hz = 0.0f;
  uint16_t length = 1;
  uint8_t velocity = 255;
};

struct Pattern {
  const PatternStep* steps = nullptr;
  uint16_t stepCount = 0;
  uint16_t unitMs = 0;
  bool loop = true;
};

class PatternTrack {
public:
  PatternTrack() = default;
  PatternTrack(INotePlayer& player, int voiceIndex, NoteProgramRef program, const Pattern& pattern);

  void bind(INotePlayer& player, int voiceIndex, NoteProgramRef program, const Pattern& pattern);
  void bindPattern(const Pattern& pattern, bool preserveTiming = true);
  void setUnitMsOverride(uint16_t unitMs) { unitMsOverride = unitMs; }
  void advanceSamples(uint32_t sampleCount);
  void reset();
  void tick();
  bool finished() const { return completed; }

private:
  void resetSequence(bool preserveRemainder);
  void advance();

  INotePlayer* notePlayer = nullptr;
  NoteProgramRef programRef{};
  const Pattern* patternRef = nullptr;
  int voice = -1;
  uint32_t samplesRemaining = 0u;
  uint16_t stepIndex = 0u;
  bool completed = false;
  uint16_t sampleRemainder = 0u;
  uint16_t unitMsOverride = 0u;
};

}  // namespace SGFAudio
