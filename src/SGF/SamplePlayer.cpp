#include "SamplePlayer.h"

#include <math.h>
#include <string.h>

namespace SGFAudio {

namespace {

float clampSample(float value) {
  if (value > 1.0f) {
    return 1.0f;
  }
  if (value < -1.0f) {
    return -1.0f;
  }
  return value;
}

}  // namespace

SamplePlayer::SamplePlayer(uint32_t sampleRate) {
  setSampleRate(sampleRate);
  reset();
}

void SamplePlayer::reset() {
  memset(voices, 0, sizeof(voices));
  nextAllocVoice = 0;
}

void SamplePlayer::setSampleRate(uint32_t sampleRate) {
  sampleRateHz = (sampleRate == 0u) ? 22050u : sampleRate;
}

void SamplePlayer::noteOn(int voiceIndex, NoteProgramRef program, float baseHz, uint8_t velocity) {
  if (voiceIndex < 0 || voiceIndex >= MAX_VOICES || program.kind != NoteProgramKind::Sample || program.ptr == nullptr) {
    return;
  }
  const SampleInstrument& instrument = *(const SampleInstrument*)(program.ptr);
  if (instrument.sample == nullptr || instrument.sample->pcm == nullptr || instrument.sample->length == 0u ||
      instrument.sample->sampleRate == 0u) {
    return;
  }
  Voice& voice = voices[voiceIndex];
  voice.active = true;
  voice.instrument = &instrument;
  voice.baseHz = (baseHz > 0.0f) ? baseHz : instrument.sample->rootHz;
  voice.samplePos = 0.0f;
  voice.velocity = velocity;
}

void SamplePlayer::noteOff(int voiceIndex) {
  if (voiceIndex < 0 || voiceIndex >= MAX_VOICES) {
    return;
  }
  Voice& voice = voices[voiceIndex];
  if (!voice.active) {
    return;
  }
  if (voice.instrument != nullptr && voice.instrument->oneShot) {
    return;
  }
  voice.active = false;
}

bool SamplePlayer::voiceActive(int voiceIndex) const {
  return voiceIndex >= 0 && voiceIndex < MAX_VOICES && voices[voiceIndex].active;
}

void SamplePlayer::playOneShot(int voiceIndex, const SampleInstrument& instrument, uint8_t velocity) {
  noteOn(voiceIndex, makeProgramRef(instrument), instrument.sample != nullptr ? instrument.sample->rootHz : 1.0f, velocity);
}

int SamplePlayer::playOneShot(const SampleInstrument& instrument, uint8_t velocity) {
  int voiceIndex = findFreeVoice();
  if (voiceIndex < 0) {
    voiceIndex = findOldestVoice();
  }
  if (voiceIndex < 0) {
    voiceIndex = 0;
  }
  nextAllocVoice = (voiceIndex + 1) % MAX_VOICES;
  playOneShot(voiceIndex, instrument, velocity);
  return voiceIndex;
}

float SamplePlayer::samplePlayback(Voice& voice, uint32_t sampleRateHz) {
  const SampleInstrument* instrument = voice.instrument;
  const AudioSample* sample = (instrument != nullptr) ? instrument->sample : nullptr;
  if (sample == nullptr || sample->pcm == nullptr || sample->length == 0u || sample->sampleRate == 0u) {
    voice.active = false;
    return 0.0f;
  }

  uint32_t sampleIndex = voice.samplePos;
  if (sampleIndex >= sample->length) {
    if (sample->loop && sample->loopEnd > sample->loopStart && sample->loopEnd <= sample->length) {
      const uint32_t loopLen = sample->loopEnd - sample->loopStart;
      sampleIndex = sample->loopStart + ((sampleIndex - sample->loopStart) % loopLen);
      voice.samplePos = sampleIndex;
    } else {
      voice.active = false;
      return 0.0f;
    }
  }

  float raw = sample->pcm[sampleIndex] / 128.0f;
  if (!sample->loop && SGF_SAMPLE_TAIL_FADE_SAMPLES > 0u) {
    const uint32_t remaining = sample->length - sampleIndex;
    if (remaining < SGF_SAMPLE_TAIL_FADE_SAMPLES) {
      const float fadeScale = remaining;
      raw *= fadeScale / SGF_SAMPLE_TAIL_FADE_SAMPLES;
    }
  }
  const float rootHz = (sample->rootHz > 0.0f) ? sample->rootHz : 440.0f;
  const float sampleRateScale = sample->sampleRate;
  const float step = (sampleRateScale / sampleRateHz) *
                     ((voice.baseHz > 0.0f ? voice.baseHz : rootHz) / rootHz);
  voice.samplePos += (step > 0.0f) ? step : 0.0f;
  return raw;
}

int16_t SamplePlayer::renderSample() {
  float mixed = 0.0f;
  for (int i = 0; i < MAX_VOICES; ++i) {
    Voice& voice = voices[i];
    if (!voice.active) {
      continue;
    }
    float raw = samplePlayback(voice, sampleRateHz);
    if (!voice.active) {
      continue;
    }
    const float velocityGain = voice.velocity / 255.0f;
    const float instrumentGain =
      (voice.instrument != nullptr ? voice.instrument->volume : 255u) / 255.0f;
    mixed += raw * velocityGain * instrumentGain;
  }
  return lrintf(clampSample(mixed) * 32767.0f);
}

void SamplePlayer::renderMono(int16_t* samples, size_t sampleCount) {
  if (samples == nullptr) {
    return;
  }
  for (size_t i = 0; i < sampleCount; ++i) {
    samples[i] = renderSample();
  }
}

int SamplePlayer::findFreeVoice() const {
  for (int offset = 0; offset < MAX_VOICES; ++offset) {
    const int voiceIndex = (nextAllocVoice + offset) % MAX_VOICES;
    if (!voices[voiceIndex].active) {
      return voiceIndex;
    }
  }
  return -1;
}

int SamplePlayer::findOldestVoice() const {
  int bestIndex = -1;
  float bestPos = -1.0f;
  for (int i = 0; i < MAX_VOICES; ++i) {
    if (!voices[i].active) {
      continue;
    }
    if (bestIndex < 0 || voices[i].samplePos > bestPos) {
      bestIndex = i;
      bestPos = voices[i].samplePos;
    }
  }
  return bestIndex;
}

}  // namespace SGFAudio
