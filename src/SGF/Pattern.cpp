#include "Pattern.h"

namespace SGFAudio {

namespace {

uint32_t unitsToSamples(uint32_t sampleRate, uint16_t unitMs, uint16_t length, uint16_t& remainderMs) {
  if (sampleRate == 0u || unitMs == 0u || length == 0u) {
    return 1u;
  }
  const uint32_t durationMs = static_cast<uint32_t>(unitMs) * static_cast<uint32_t>(length);
  const uint64_t numerator =
    static_cast<uint64_t>(sampleRate) * static_cast<uint64_t>(durationMs) + remainderMs;
  const uint32_t samples = static_cast<uint32_t>(numerator / 1000u);
  remainderMs = static_cast<uint16_t>(numerator % 1000u);
  return samples == 0u ? 1u : samples;
}

}  // namespace

PatternTrack::PatternTrack(
  SynthEngine& synth, int voiceIndex, const Instrument& instrument, const Pattern& pattern) {
  bind(synth, voiceIndex, instrument, pattern);
}

void PatternTrack::bind(
  SynthEngine& synth, int voiceIndex, const Instrument& instrument, const Pattern& pattern) {
  synthEngine = &synth;
  instrumentRef = &instrument;
  patternRef = &pattern;
  voice = voiceIndex;
  reset();
}

void PatternTrack::bindPattern(const Pattern& pattern, bool preserveTiming) {
  patternRef = &pattern;
  resetSequence(preserveTiming);
}

void PatternTrack::reset() {
  resetSequence(false);
  if (synthEngine != nullptr && voice >= 0) {
    synthEngine->noteOff(voice);
  }
}

void PatternTrack::resetSequence(bool preserveRemainder) {
  samplesRemaining = 0u;
  stepIndex = 0u;
  completed = false;
  if (!preserveRemainder) {
    sampleRemainder = 0u;
  }
}

void PatternTrack::tick() {
  if (synthEngine == nullptr || instrumentRef == nullptr || patternRef == nullptr ||
      patternRef->steps == nullptr || patternRef->stepCount == 0u || voice < 0) {
    return;
  }
  if (completed) {
    return;
  }
  if (samplesRemaining == 0u) {
    advance();
  }
  if (samplesRemaining > 0u) {
    --samplesRemaining;
  }
}

void PatternTrack::advance() {
  if (synthEngine == nullptr || instrumentRef == nullptr || patternRef == nullptr ||
      patternRef->steps == nullptr || patternRef->stepCount == 0u || voice < 0) {
    return;
  }

  if (stepIndex >= patternRef->stepCount) {
    if (!patternRef->loop) {
      synthEngine->noteOff(voice);
      samplesRemaining = 0u;
      completed = true;
      return;
    }
    stepIndex = 0u;
  }

  const PatternStep& step = patternRef->steps[stepIndex];
  if (step.hz > 0.0f && step.velocity > 0u) {
    synthEngine->noteOn(voice, *instrumentRef, step.hz, step.velocity);
  } else {
    synthEngine->noteOff(voice);
  }

  samplesRemaining = unitsToSamples(
    synthEngine->sampleRate(), patternRef->unitMs, step.length, sampleRemainder);
  ++stepIndex;
}

}  // namespace SGFAudio
