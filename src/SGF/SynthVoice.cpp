#include "Synth.h"

#include <math.h>

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
  voice.noiseState = 0xA341316Cu ^ static_cast<uint32_t>(lrintf(voice.baseHz * 100.0f));
  voice.samplePos = 0.0f;
  voice.filter.lowPassState = 0.0f;
  voice.filter.highPassLowState = 0.0f;
  updateFilterCoefficients(voice);
  retriggerVoice(voice);
}

void SynthEngine::retriggerVoice(Voice& voice) {
  voice.phase = 0.0f;
  voice.lfoPhase = 0.0f;
  voice.ageSamples = 0u;
  voice.samplePos = 0.0f;
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
  const float pitchRatio = semitoneRatio(pitchCents / 100.0f);
  voice.currentHz = voice.baseHz * pitchRatio;
  const float phaseStep = voice.currentHz / static_cast<float>(sampleRateHz);
  float raw = 0.0f;
  if (voice.instrument->sample != nullptr) {
    raw = samplePlayback(voice, pitchRatio);
    if (!voice.active) {
      return 0.0f;
    }
  } else if (voice.instrument->waveform == Waveform::Noise) {
    raw = noiseSample(voice);
  } else {
    raw = waveformSample(voice.instrument->waveform, voice.phase);
    voice.phase += phaseStep;
    if (voice.phase >= 1.0f) {
      voice.phase -= floorf(voice.phase);
    }
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

float SynthEngine::samplePlayback(Voice& voice, float pitchRatio) const {
  const AudioSample* sample = voice.instrument->sample;
  if (sample == nullptr || sample->pcm == nullptr || sample->length == 0u || sample->sampleRate == 0u) {
    voice.active = false;
    voice.envStage = EnvStage::Idle;
    return 0.0f;
  }

  uint32_t sampleIndex = static_cast<uint32_t>(voice.samplePos);
  if (sampleIndex >= sample->length) {
    if (sample->loop && sample->loopEnd > sample->loopStart && sample->loopEnd <= sample->length) {
      const uint32_t loopLen = sample->loopEnd - sample->loopStart;
      sampleIndex = sample->loopStart + ((sampleIndex - sample->loopStart) % loopLen);
      voice.samplePos = static_cast<float>(sampleIndex);
    } else {
      voice.active = false;
      voice.envStage = EnvStage::Idle;
      return 0.0f;
    }
  }

  const float raw = static_cast<float>(sample->pcm[sampleIndex]) / 128.0f;
  const float rootHz = (sample->rootHz > 0.0f) ? sample->rootHz : 440.0f;
  const float step = (static_cast<float>(sample->sampleRate) / static_cast<float>(sampleRateHz)) *
                     (voice.baseHz / rootHz) * pitchRatio;
  voice.samplePos += (step > 0.0f) ? step : 0.0f;
  return raw;
}

}  // namespace SGFAudio
