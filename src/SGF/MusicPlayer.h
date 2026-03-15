#pragma once

#include <stdint.h>

#include "AudioMixer.h"
#include "IAudioSource.h"
#include "INotePlayer.h"
#include "SamplePlayer.h"
#include "Song.h"
#include "Synth.h"

namespace SGFAudio {

class MusicPlayer : public IAudioSource, public INotePlayer {
public:
  explicit MusicPlayer(uint32_t sampleRate = 22050u);

  void play(const Song& song, bool restart = true);
  void stop(uint16_t fadeOutMs = 0u);

  void setVolume(uint8_t volume, uint16_t fadeMs = 0u);
  uint8_t volume() const { return steadyVolume; }

  SynthEngine& synth() { return synthEngine; }
  SamplePlayer& samples() { return samplePlayer; }
  const SynthEngine& synth() const { return synthEngine; }
  const SamplePlayer& samples() const { return samplePlayer; }

  uint32_t sampleRate() const override { return mixer.sampleRate(); }
  void noteOn(int voiceIndex, NoteProgramRef program, float baseHz, uint8_t velocity = 255) override;
  void noteOff(int voiceIndex) override;
  bool voiceActive(int voiceIndex) const override;
  int16_t renderSample() override;

private:
  void startFade(uint8_t targetVolume, uint16_t fadeMs);
  void finishStop();

  AudioMixer mixer;
  SynthEngine synthEngine;
  SamplePlayer samplePlayer;
  SongPlayer songPlayer;
  const Song* currentSong = nullptr;
  bool sequenceActive = false;
  bool stopPending = false;
  uint8_t steadyVolume = 255u;
  int32_t gainCurrent = 255;
  int32_t gainStart = 255;
  int32_t gainTarget = 255;
  uint32_t gainSamples = 0u;
  uint32_t gainSamplesRemaining = 0u;
};

}  // namespace SGFAudio
