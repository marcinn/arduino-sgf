#include "Synth.h"

#include <math.h>

namespace SGFAudio {

namespace {

constexpr float PI_F = 3.14159265358979323846f;

}  // namespace

float SynthEngine::waveformSample(Waveform waveform, float phase) {
  switch (waveform) {
    case Waveform::Sine:
      return sinf(phase * 2.0f * PI_F);
    case Waveform::Triangle:
      return 1.0f - 4.0f * fabsf(phase - 0.5f);
    case Waveform::Square:
      return (phase < 0.5f) ? 1.0f : -1.0f;
    case Waveform::Saw:
      return (2.0f * phase) - 1.0f;
    case Waveform::Noise:
    default:
      return 0.0f;
  }
}

float SynthEngine::noiseSample(Voice& voice) {
  voice.noiseState = voice.noiseState * 1664525u + 1013904223u;
  const uint32_t bits = (voice.noiseState >> 8) & 0xFFFFu;
  return (bits / 32767.5f) - 1.0f;
}

float SynthEngine::clampUnit(float value) {
  if (value < 0.0f) {
    return 0.0f;
  }
  if (value > 1.0f) {
    return 1.0f;
  }
  return value;
}

float SynthEngine::semitoneRatio(float semitones) {
  return powf(2.0f, semitones / 12.0f);
}

uint32_t SynthEngine::msToSamples(uint32_t sampleRate, uint16_t ms) {
  if (ms == 0u || sampleRate == 0u) {
    return 0u;
  }
  const uint64_t sampleProduct = (uint64_t)sampleRate * ms;
  const uint32_t samples = sampleProduct / 1000u;
  return samples == 0u ? 1u : samples;
}

}  // namespace SGFAudio
