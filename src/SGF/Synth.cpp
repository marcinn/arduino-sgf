#include "Synth.h"

#include <math.h>
#include <string.h>

namespace SGFAudio {

namespace {

constexpr float PI_F = 3.14159265358979323846f;

float cutoffAlpha(float cutoffHz, uint32_t sampleRate) {
  if (cutoffHz <= 0.0f || sampleRate == 0u) {
    return 0.0f;
  }
  const float omega = 2.0f * PI_F * cutoffHz / static_cast<float>(sampleRate);
  return expf(-omega);
}

}  // namespace

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
  voice.releaseStep = voice.envelope / static_cast<float>(releaseSamples == 0u ? 1u : releaseSamples);
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
    mixed /= static_cast<float>(activeVoices);
  }
  mixed *= static_cast<float>(masterGain) / 255.0f;
  if (mixed > 1.0f) {
    mixed = 1.0f;
  } else if (mixed < -1.0f) {
    mixed = -1.0f;
  }
  return static_cast<int16_t>(lrintf(mixed * 32767.0f));
}

void SynthEngine::renderMono(int16_t* samples, size_t sampleCount) {
  if (samples == nullptr) {
    return;
  }
  for (size_t i = 0; i < sampleCount; ++i) {
    samples[i] = renderSample();
  }
}

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
    default:
      return 0.0f;
  }
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
  const uint32_t samples = static_cast<uint32_t>((static_cast<uint64_t>(sampleRate) * ms) / 1000u);
  return samples == 0u ? 1u : samples;
}

void SynthEngine::startVoice(Voice& voice, const Instrument& instrument, float baseHz, uint8_t velocity) {
  voice.active = true;
  voice.instrument = &instrument;
  voice.baseHz = baseHz > 0.0f ? baseHz : 440.0f;
  voice.phase = 0.0f;
  voice.lfoPhase = 0.0f;
  voice.velocity = velocity;
  voice.semitoneOffset = 0;
  voice.centsOffset = 0;
  voice.stepVolume = 255;
  voice.sfx = nullptr;
  voice.stepIndex = 0u;
  voice.stepElapsedSamples = 0u;
  voice.stepDurationSamples = 0u;
  voice.ageSamples = 0u;
  voice.filter.lowPassState = 0.0f;
  voice.filter.highPassLowState = 0.0f;
  updateFilterCoefficients(voice);
  retriggerVoice(voice);
}

void SynthEngine::retriggerVoice(Voice& voice) {
  voice.phase = 0.0f;
  voice.lfoPhase = 0.0f;
  voice.ageSamples = 0u;
  voice.envelope = 0.0f;
  voice.releaseStep = 0.0f;
  const Adsr& adsr = voice.instrument->ampEnv;
  if (adsr.attackMs == 0u) {
    voice.envelope = 1.0f;
    voice.envStage = (adsr.decayMs == 0u && adsr.sustain == 255u) ? EnvStage::Sustain : EnvStage::Decay;
    if (adsr.decayMs == 0u) {
      voice.envelope = static_cast<float>(adsr.sustain) / 255.0f;
      voice.envStage = EnvStage::Sustain;
    }
  } else {
    voice.envStage = EnvStage::Attack;
  }
}

void SynthEngine::updateFilterCoefficients(Voice& voice) {
  if (voice.instrument == nullptr) {
    voice.filter.lowPassAlpha = 0.0f;
    voice.filter.highPassAlpha = 0.0f;
    return;
  }
  voice.filter.lowPassAlpha = cutoffAlpha(voice.instrument->lowPassCutoffHz, sampleRateHz);
  voice.filter.highPassAlpha = cutoffAlpha(voice.instrument->highPassCutoffHz, sampleRateHz);
}

void SynthEngine::applySfxStep(Voice& voice, const SfxStep& step) {
  voice.semitoneOffset = step.semitoneOffset;
  voice.centsOffset = step.centsOffset;
  voice.stepVolume = step.volume;
  voice.stepElapsedSamples = 0u;
  voice.stepDurationSamples = msToSamples(sampleRateHz, step.durationMs);
  if (!step.gate) {
    noteOff(static_cast<int>(&voice - voices));
    return;
  }
  if (step.retrigger) {
    retriggerVoice(voice);
  }
}

void SynthEngine::advanceSfx(Voice& voice) {
  if (voice.sfx == nullptr || voice.stepDurationSamples == 0u) {
    return;
  }
  ++voice.stepElapsedSamples;
  if (voice.stepElapsedSamples < voice.stepDurationSamples) {
    return;
  }
  ++voice.stepIndex;
  if (voice.stepIndex >= voice.sfx->stepCount) {
    voice.sfx = nullptr;
    noteOff(static_cast<int>(&voice - voices));
    return;
  }
  applySfxStep(voice, voice.sfx->steps[voice.stepIndex]);
}

float SynthEngine::pitchEnvCents(const Voice& voice) const {
  if (voice.instrument == nullptr || voice.instrument->pitchEnv == nullptr ||
      voice.instrument->pitchEnvCount == 0u) {
    return 0.0f;
  }
  if (voice.instrument->pitchEnvCount == 1u) {
    return static_cast<float>(voice.instrument->pitchEnv[0].cents);
  }
  const float ageMs = (static_cast<float>(voice.ageSamples) * 1000.0f) / static_cast<float>(sampleRateHz);
  const PitchPoint* points = voice.instrument->pitchEnv;
  if (ageMs <= static_cast<float>(points[0].timeMs)) {
    return static_cast<float>(points[0].cents);
  }
  for (uint8_t i = 1; i < voice.instrument->pitchEnvCount; ++i) {
    if (ageMs <= static_cast<float>(points[i].timeMs)) {
      const float t0 = static_cast<float>(points[i - 1].timeMs);
      const float t1 = static_cast<float>(points[i].timeMs);
      const float a = (ageMs - t0) / ((t1 - t0) <= 0.0f ? 1.0f : (t1 - t0));
      const float c0 = static_cast<float>(points[i - 1].cents);
      const float c1 = static_cast<float>(points[i].cents);
      return c0 + (c1 - c0) * a;
    }
  }
  return static_cast<float>(points[voice.instrument->pitchEnvCount - 1].cents);
}

float SynthEngine::lfoCents(Voice& voice) const {
  if (voice.instrument == nullptr || !voice.instrument->pitchLfo.enabled ||
      voice.instrument->pitchLfo.depthCents == 0.0f || voice.instrument->pitchLfo.rateHz <= 0.0f) {
    return 0.0f;
  }
  const float sample = waveformSample(voice.instrument->pitchLfo.waveform, voice.lfoPhase);
  voice.lfoPhase += voice.instrument->pitchLfo.rateHz / static_cast<float>(sampleRateHz);
  if (voice.lfoPhase >= 1.0f) {
    voice.lfoPhase -= floorf(voice.lfoPhase);
  }
  return sample * voice.instrument->pitchLfo.depthCents;
}

float SynthEngine::nextVoiceSample(Voice& voice) {
  if (!voice.active || voice.instrument == nullptr || voice.envStage == EnvStage::Idle) {
    voice.active = false;
    return 0.0f;
  }

  advanceSfx(voice);
  const float pitchCents = static_cast<float>(voice.semitoneOffset * 100) +
                           static_cast<float>(voice.centsOffset) +
                           pitchEnvCents(voice) + lfoCents(voice);
  voice.currentHz = voice.baseHz * semitoneRatio(pitchCents / 100.0f);
  const float phaseStep = voice.currentHz / static_cast<float>(sampleRateHz);
  const float raw = waveformSample(voice.instrument->waveform, voice.phase);
  voice.phase += phaseStep;
  if (voice.phase >= 1.0f) {
    voice.phase -= floorf(voice.phase);
  }

  const float velocityGain = static_cast<float>(voice.velocity) / 255.0f;
  const float stepGain = static_cast<float>(voice.stepVolume) / 255.0f;
  const float instrumentGain = static_cast<float>(voice.instrument->volume) / 255.0f;
  float sample = raw * voice.envelope * velocityGain * stepGain * instrumentGain;
  sample = applyFilters(voice, sample);
  advanceEnvelope(voice);
  ++voice.ageSamples;

  if (voice.envStage == EnvStage::Idle) {
    voice.active = false;
    return 0.0f;
  }
  return sample;
}

void SynthEngine::advanceEnvelope(Voice& voice) {
  if (voice.instrument == nullptr) {
    voice.envStage = EnvStage::Idle;
    voice.envelope = 0.0f;
    return;
  }
  const Adsr& adsr = voice.instrument->ampEnv;
  switch (voice.envStage) {
    case EnvStage::Attack: {
      const uint32_t attackSamples = msToSamples(sampleRateHz, adsr.attackMs);
      voice.envelope += 1.0f / static_cast<float>(attackSamples == 0u ? 1u : attackSamples);
      if (voice.envelope >= 1.0f) {
        voice.envelope = 1.0f;
        if (adsr.decayMs == 0u) {
          voice.envelope = static_cast<float>(adsr.sustain) / 255.0f;
          voice.envStage = EnvStage::Sustain;
        } else {
          voice.envStage = EnvStage::Decay;
        }
      }
      break;
    }
    case EnvStage::Decay: {
      const float sustain = static_cast<float>(adsr.sustain) / 255.0f;
      const uint32_t decaySamples = msToSamples(sampleRateHz, adsr.decayMs);
      const float step = (1.0f - sustain) / static_cast<float>(decaySamples == 0u ? 1u : decaySamples);
      voice.envelope -= step;
      if (voice.envelope <= sustain) {
        voice.envelope = sustain;
        voice.envStage = EnvStage::Sustain;
      }
      break;
    }
    case EnvStage::Sustain:
      voice.envelope = static_cast<float>(adsr.sustain) / 255.0f;
      break;
    case EnvStage::Release:
      voice.envelope -= voice.releaseStep;
      if (voice.envelope <= 0.0f) {
        voice.envelope = 0.0f;
        voice.envStage = EnvStage::Idle;
      }
      break;
    case EnvStage::Idle:
    default:
      voice.envelope = 0.0f;
      break;
  }
  voice.envelope = clampUnit(voice.envelope);
}

float SynthEngine::applyFilters(Voice& voice, float sample) {
  if (voice.instrument == nullptr) {
    return sample;
  }
  float filtered = sample;
  if ((voice.instrument->filterFlags & FilterLowPass) != 0u && voice.instrument->lowPassCutoffHz > 0.0f) {
    const float alpha = voice.filter.lowPassAlpha;
    voice.filter.lowPassState = (1.0f - alpha) * filtered + alpha * voice.filter.lowPassState;
    filtered = voice.filter.lowPassState;
  }
  if ((voice.instrument->filterFlags & FilterHighPass) != 0u && voice.instrument->highPassCutoffHz > 0.0f) {
    const float alpha = voice.filter.highPassAlpha;
    voice.filter.highPassLowState =
      (1.0f - alpha) * filtered + alpha * voice.filter.highPassLowState;
    filtered -= voice.filter.highPassLowState;
  }
  return filtered;
}

}  // namespace SGFAudio
