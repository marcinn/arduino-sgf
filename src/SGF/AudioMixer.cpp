#include "AudioMixer.h"

#include <math.h>

namespace SGFAudio {

AudioMixer::AudioMixer(uint32_t sampleRate)
  : sampleRateHz(sampleRate == 0u ? 22050u : sampleRate) {}

bool AudioMixer::addSource(IAudioSource& source) {
  if (source.sampleRate() != sampleRateHz || sourceCount >= MAX_SOURCES) {
    return false;
  }
  sources[sourceCount++] = &source;
  return true;
}

void AudioMixer::clearSources() {
  for (uint8_t i = 0u; i < sourceCount; ++i) {
    sources[i] = nullptr;
  }
  sourceCount = 0u;
}

void AudioMixer::setMasterVolume(uint8_t volume) {
  masterGain = volume;
}

void AudioMixer::advanceSamples(uint32_t sampleCount) {
  for (uint8_t i = 0u; i < sourceCount; ++i) {
    if (sources[i] == nullptr) {
      continue;
    }
    sources[i]->advanceSamples(sampleCount);
  }
}

int16_t AudioMixer::renderSample() {
  float mixed = 0.0f;
  for (uint8_t i = 0u; i < sourceCount; ++i) {
    if (sources[i] == nullptr) {
      continue;
    }
    mixed += sources[i]->renderSample() / 32767.0f;
  }
  mixed *= masterGain / 255.0f;
  if (mixed > 1.0f) {
    mixed = 1.0f;
  } else if (mixed < -1.0f) {
    mixed = -1.0f;
  }
  return lrintf(mixed * 32767.0f);
}

void AudioMixer::renderMono(int16_t* samples, size_t sampleCount) {
  if (samples == nullptr) {
    return;
  }
  for (size_t i = 0; i < sampleCount; ++i) {
    samples[i] = renderSample();
  }
}

}  // namespace SGFAudio
