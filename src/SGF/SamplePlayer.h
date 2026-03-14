#pragma once

#include <stddef.h>
#include <stdint.h>

#include "IAudioSource.h"
#include "INotePlayer.h"

namespace SGFAudio {

class SamplePlayer : public IAudioSource, public INotePlayer {
public:
  static constexpr int MAX_VOICES = 8;

  explicit SamplePlayer(uint32_t sampleRate = 22050u);

  void reset();
  void setSampleRate(uint32_t sampleRate);
  uint32_t sampleRate() const override { return sampleRateHz; }

  void noteOn(int voiceIndex, NoteProgramRef program, float baseHz, uint8_t velocity = 255) override;
  void noteOff(int voiceIndex) override;
  bool voiceActive(int voiceIndex) const override;

  void playOneShot(int voiceIndex, const SampleInstrument& instrument, uint8_t velocity = 255);
  int playOneShot(const SampleInstrument& instrument, uint8_t velocity = 255);

  int16_t renderSample() override;
  void renderMono(int16_t* samples, size_t sampleCount);

private:
  struct Voice {
    bool active = false;
    const SampleInstrument* instrument = nullptr;
    float baseHz = 0.0f;
    float samplePos = 0.0f;
    uint8_t velocity = 255;
  };

  uint32_t sampleRateHz = 22050u;
  Voice voices[MAX_VOICES]{};

  static float samplePlayback(Voice& voice, uint32_t sampleRateHz);
};

}  // namespace SGFAudio
