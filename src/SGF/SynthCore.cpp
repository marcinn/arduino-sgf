#include "Synth.h"

#include <math.h>
#include <string.h>

namespace SGFAudio {

SynthEngine::SynthEngine(uint32_t sampleRate) {
  setSampleRate(sampleRate);
  reset();
}

void SynthEngine::setSampleRate(uint32_t sampleRate) {
  sampleRateHz = (sampleRate == 0u) ? 22050u : sampleRate;
  for (int i = 0; i < MAX_VOICES; ++i) {
    updateFilterCoefficients(voices[i]);
  }
}

void SynthEngine::setMasterVolume(uint8_t volume) {
  masterGain = volume;
}

void SynthEngine::reset() {
  memset(voices, 0, sizeof(voices));
}

void SynthEngine::noteOn(int voiceIndex, NoteProgramRef program, float baseHz, uint8_t velocity) {
  if (program.kind != NoteProgramKind::Synth || program.ptr == nullptr) {
    return;
  }
  noteOn(voiceIndex, *(const Instrument*)(program.ptr), baseHz, velocity);
}

void SynthEngine::noteOn(int voiceIndex, const Instrument& instrument, float baseHz, uint8_t velocity) {
  if (voiceIndex < 0 || voiceIndex >= MAX_VOICES) {
    return;
  }
  startVoice(voices[voiceIndex], instrument, baseHz, velocity);
}

void SynthEngine::noteOff(int voiceIndex) {
  if (voiceIndex < 0 || voiceIndex >= MAX_VOICES) {
    return;
  }
  Voice& voice = voices[voiceIndex];
  if (!voice.active) {
    return;
  }
  if (voice.instrument == nullptr) {
    voice.active = false;
    voice.envStage = EnvStage::Idle;
    return;
  }
  voice.sfx = nullptr;
  voice.stepDurationSamples = 0u;
  voice.stepElapsedSamples = 0u;
  if (voice.instrument->ampEnv.releaseMs == 0u || voice.envelope <= 0.0f) {
    voice.active = false;
    voice.envStage = EnvStage::Idle;
    voice.envelope = 0.0f;
    return;
  }
  const uint32_t releaseSamples = msToSamples(sampleRateHz, voice.instrument->ampEnv.releaseMs);
  const float releaseSamplesF = releaseSamples == 0u ? 1.0f : releaseSamples;
  voice.releaseStep = voice.envelope / releaseSamplesF;
  voice.envStage = EnvStage::Release;
}

void SynthEngine::triggerSfx(int voiceIndex, const Sfx& sfx, float baseHz, uint8_t velocity) {
  if (voiceIndex < 0 || voiceIndex >= MAX_VOICES || sfx.instrument == nullptr ||
      sfx.steps == nullptr || sfx.stepCount == 0u) {
    return;
  }
  Voice& voice = voices[voiceIndex];
  startVoice(voice, *sfx.instrument, baseHz, velocity);
  voice.sfx = &sfx;
  voice.stepIndex = 0u;
  voice.stepElapsedSamples = 0u;
  applySfxStep(voice, sfx.steps[0]);
}

int SynthEngine::playSfx(const Sfx& sfx, float baseHz, uint8_t velocity) {
  for (int i = 0; i < MAX_VOICES; ++i) {
    if (!voices[i].active) {
      triggerSfx(i, sfx, baseHz, velocity);
      return i;
    }
  }
  triggerSfx(0, sfx, baseHz, velocity);
  return 0;
}

bool SynthEngine::voiceActive(int voiceIndex) const {
  return voiceIndex >= 0 && voiceIndex < MAX_VOICES && voices[voiceIndex].active;
}

int16_t SynthEngine::renderSample() {
  float mixed = 0.0f;
  int activeVoices = 0;
  for (int i = 0; i < MAX_VOICES; ++i) {
    if (!voices[i].active) {
      continue;
    }
    mixed += nextVoiceSample(voices[i]);
    ++activeVoices;
  }
  if (activeVoices > 1) {
    mixed /= activeVoices;
  }
  mixed *= masterGain / 255.0f;
  if (mixed > 1.0f) {
    mixed = 1.0f;
  } else if (mixed < -1.0f) {
    mixed = -1.0f;
  }
  return lrintf(mixed * 32767.0f);
}

void SynthEngine::renderMono(int16_t* samples, size_t sampleCount) {
  if (samples == nullptr) {
    return;
  }
  for (size_t i = 0; i < sampleCount; ++i) {
    samples[i] = renderSample();
  }
}

}  // namespace SGFAudio
