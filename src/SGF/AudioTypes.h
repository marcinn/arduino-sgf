#pragma once

#include <stdint.h>

namespace SGFAudio {

enum class Waveform : uint8_t {
  Sine = 0,
  Triangle,
  Square,
  Saw,
  Noise,
};

#define AUDIO_FILTER_LP (1u << 0)
#define AUDIO_FILTER_HP (1u << 1)

enum class NoteProgramKind : uint8_t {
  Synth = 0,
  Sample,
};

struct Adsr {
  uint16_t attackMs = 0;
  uint16_t decayMs = 0;
  uint8_t sustain = 255;
  uint16_t releaseMs = 0;
};

struct Lfo {
  bool enabled = false;
  Waveform waveform = Waveform::Sine;
  float rateHz = 0.0f;
  float depthCents = 0.0f;
};

struct PitchPoint {
  uint16_t timeMs = 0;
  int16_t cents = 0;
};

struct AudioSample {
  const int8_t* pcm = nullptr;
  uint32_t length = 0;
  uint32_t sampleRate = 0;
  float rootHz = 440.0f;
  bool loop = false;
  uint32_t loopStart = 0;
  uint32_t loopEnd = 0;
};

struct Instrument {
  Waveform waveform = Waveform::Square;
  Adsr ampEnv{};
  Lfo pitchLfo{};
  const PitchPoint* pitchEnv = nullptr;
  uint8_t pitchEnvCount = 0;
  uint8_t filterFlags = 0u;
  float lowPassCutoffHz = 0.0f;
  float highPassCutoffHz = 0.0f;
  uint8_t volume = 255;
};

struct SampleInstrument {
  const AudioSample* sample = nullptr;
  uint8_t volume = 255;
  bool oneShot = true;
};

struct NoteProgramRef {
  NoteProgramKind kind = NoteProgramKind::Synth;
  const void* ptr = nullptr;
};

inline NoteProgramRef makeProgramRef(const Instrument& instrument) {
  return NoteProgramRef{NoteProgramKind::Synth, &instrument};
}

inline NoteProgramRef makeProgramRef(const SampleInstrument& instrument) {
  return NoteProgramRef{NoteProgramKind::Sample, &instrument};
}

}  // namespace SGFAudio
