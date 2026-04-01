#pragma once

#include <stddef.h>
#include <stdint.h>

#include "AudioTypes.h"
#include "IAudioSource.h"
#include "INotePlayer.h"

namespace SGFAudio {

struct SfxStep {
  uint16_t durationMs = 0;
  int8_t semitoneOffset = 0;
  int16_t centsOffset = 0;
  uint8_t volume = 255;
  bool gate = true;
  bool retrigger = true;
};

struct Sfx {
  const Instrument* instrument = nullptr;
  const SfxStep* steps = nullptr;
  uint8_t stepCount = 0;
};

class SynthEngine : public IAudioSource, public INotePlayer {
public:
  static constexpr int MAX_VOICES = 8;

  explicit SynthEngine(uint32_t sampleRate = 22050u);

  void setSampleRate(uint32_t sampleRate);
  uint32_t sampleRate() const override { return sampleRateHz; }

  void setMasterVolume(uint8_t volume);
  uint8_t masterVolume() const { return masterGain; }

  void reset();

  void noteOn(int voiceIndex, NoteProgramRef program, float baseHz, uint8_t velocity = 255) override;
  void noteOn(int voiceIndex, const Instrument& instrument, float baseHz, uint8_t velocity = 255);
  void noteOff(int voiceIndex) override;

  void triggerSfx(int voiceIndex, const Sfx& sfx, float baseHz, uint8_t velocity = 255);
  int playSfx(const Sfx& sfx, float baseHz, uint8_t velocity = 255);

  bool voiceActive(int voiceIndex) const override;

  int16_t renderSample() override;
  void renderMono(int16_t* samples, size_t sampleCount);

private:
  enum class EnvStage : uint8_t {
    Idle = 0,
    Attack,
    Decay,
    Sustain,
    Release,
  };

  struct FilterState {
    float lowPassState = 0.0f;
    float highPassLowState = 0.0f;
    float lowPassAlpha = 0.0f;
    float highPassAlpha = 0.0f;
  };

  struct Voice {
    bool active = false;
    const Instrument* instrument = nullptr;
    float baseHz = 0.0f;
    float phase = 0.0f;
    float lfoPhase = 0.0f;
    float envelope = 0.0f;
    float releaseStep = 0.0f;
    float currentHz = 0.0f;
    uint8_t velocity = 255;
    int8_t semitoneOffset = 0;
    int16_t centsOffset = 0;
    uint8_t stepVolume = 255;
    uint32_t noiseState = 0x12345678u;
    float samplePos = 0.0f;
    EnvStage envStage = EnvStage::Idle;
    uint32_t ageSamples = 0;
    uint32_t stepElapsedSamples = 0;
    uint32_t stepDurationSamples = 0;
    uint8_t stepIndex = 0;
    const Sfx* sfx = nullptr;
    FilterState filter{};
  };

  uint32_t sampleRateHz = 22050u;
  uint8_t masterGain = 255;
  Voice voices[MAX_VOICES]{};
  int nextAllocVoice = 0;

  static float waveformSample(Waveform waveform, float phase);
  static float noiseSample(Voice& voice);
  static float clampUnit(float value);
  static float semitoneRatio(float semitones);
  static uint32_t msToSamples(uint32_t sampleRate, uint16_t ms);
  int findFreeVoice() const;
  int findOldestVoice() const;

  void startVoice(Voice& voice, const Instrument& instrument, float baseHz, uint8_t velocity);
  void retriggerVoice(Voice& voice);
  void updateFilterCoefficients(Voice& voice);
  void applySfxStep(Voice& voice, const SfxStep& step);
  void advanceSfx(Voice& voice);
  float pitchEnvCents(const Voice& voice) const;
  float lfoCents(Voice& voice) const;
  float nextVoiceSample(Voice& voice);
  void advanceEnvelope(Voice& voice);
  float applyFilters(Voice& voice, float sample);
};

}  // namespace SGFAudio
