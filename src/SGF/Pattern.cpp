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
  INotePlayer& player, int voiceIndex, NoteProgramRef program, const Pattern& pattern) {
  bind(player, voiceIndex, program, pattern);
}

void PatternTrack::bind(
  INotePlayer& player, int voiceIndex, NoteProgramRef program, const Pattern& pattern) {
  notePlayer = &player;
  programRef = program;
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
  if (notePlayer != nullptr && voice >= 0) {
    notePlayer->noteOff(voice);
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
  if (notePlayer == nullptr || programRef.ptr == nullptr || patternRef == nullptr ||
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
  if (notePlayer == nullptr || programRef.ptr == nullptr || patternRef == nullptr ||
      patternRef->steps == nullptr || patternRef->stepCount == 0u || voice < 0) {
    return;
  }

  if (stepIndex >= patternRef->stepCount) {
    if (!patternRef->loop) {
      notePlayer->noteOff(voice);
      samplesRemaining = 0u;
      completed = true;
      return;
    }
    stepIndex = 0u;
  }

  const PatternStep& step = patternRef->steps[stepIndex];
  if (step.hz > 0.0f && step.velocity > 0u) {
    notePlayer->noteOn(voice, programRef, step.hz, step.velocity);
  } else {
    notePlayer->noteOff(voice);
  }

  samplesRemaining = unitsToSamples(
    notePlayer->sampleRate(), patternRef->unitMs, step.length, sampleRemainder);
  ++stepIndex;
}

}  // namespace SGFAudio
