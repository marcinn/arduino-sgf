#pragma once

#include <stdint.h>

#include "Synth.h"

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
  PatternTrack(SynthEngine& synth, int voiceIndex, const Instrument& instrument, const Pattern& pattern);

  void bind(SynthEngine& synth, int voiceIndex, const Instrument& instrument, const Pattern& pattern);
  void bindPattern(const Pattern& pattern);
  void reset();
  void tick();
  bool finished() const { return completed; }

private:
  void advance();

  SynthEngine* synthEngine = nullptr;
  const Instrument* instrumentRef = nullptr;
  const Pattern* patternRef = nullptr;
  int voice = -1;
  uint32_t samplesRemaining = 0u;
  uint16_t stepIndex = 0u;
  bool completed = false;
};

}  // namespace SGFAudio
