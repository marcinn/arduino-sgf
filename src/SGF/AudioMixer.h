#pragma once

#include <stddef.h>
#include <stdint.h>

#include "IAudioSource.h"

namespace SGFAudio {

class AudioMixer : public IAudioSource {
public:
  static constexpr uint8_t MAX_SOURCES = 4;

  explicit AudioMixer(uint32_t sampleRate = 22050u);

  bool addSource(IAudioSource& source);
  void clearSources();

  void setMasterVolume(uint8_t volume);
  uint8_t masterVolume() const { return masterGain; }

  uint32_t sampleRate() const override { return sampleRateHz; }
  int16_t renderSample() override;
  void renderMono(int16_t* samples, size_t sampleCount);

private:
  uint32_t sampleRateHz = 22050u;
  uint8_t masterGain = 255u;
  IAudioSource* sources[MAX_SOURCES]{};
  uint8_t sourceCount = 0u;
};

}  // namespace SGFAudio
