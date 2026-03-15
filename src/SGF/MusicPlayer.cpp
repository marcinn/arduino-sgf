#include "MusicPlayer.h"

namespace SGFAudio {

namespace {

uint32_t msToSamples(uint32_t sampleRate, uint16_t ms) {
  if (sampleRate == 0u || ms == 0u) {
    return 0u;
  }
  const uint32_t samples = (sampleRate * ms) / 1000u;
  return samples > 0u ? samples : 1u;
}

}  // namespace

MusicPlayer::MusicPlayer(uint32_t sampleRate)
  : mixer(sampleRate),
    synthEngine(sampleRate),
    samplePlayer(sampleRate),
    songPlayer() {
  mixer.addSource(synthEngine);
  mixer.addSource(samplePlayer);
}

void MusicPlayer::play(const Song& song, bool restart) {
  if (!restart && currentSong == &song && sequenceActive) {
    return;
  }
  currentSong = &song;
  stopPending = false;
  songPlayer.bind(song, *this);
  sequenceActive = true;
  gainCurrent = steadyVolume;
  gainStart = steadyVolume;
  gainTarget = steadyVolume;
  gainSamples = 0u;
  gainSamplesRemaining = 0u;
}

void MusicPlayer::stop(uint16_t fadeOutMs) {
  songPlayer.reset();
  sequenceActive = false;
  if (fadeOutMs == 0u) {
    finishStop();
    return;
  }
  stopPending = true;
  startFade(0u, fadeOutMs);
}

void MusicPlayer::setVolume(uint8_t volume, uint16_t fadeMs) {
  steadyVolume = volume;
  stopPending = false;
  startFade(volume, fadeMs);
}

void MusicPlayer::noteOn(int voiceIndex, NoteProgramRef program, float baseHz, uint8_t velocity) {
  if (program.ptr == nullptr) {
    return;
  }
  if (program.kind == NoteProgramKind::Sample) {
    samplePlayer.noteOn(voiceIndex, program, baseHz, velocity);
    synthEngine.noteOff(voiceIndex);
    return;
  }
  synthEngine.noteOn(voiceIndex, program, baseHz, velocity);
  samplePlayer.noteOff(voiceIndex);
}

void MusicPlayer::noteOff(int voiceIndex) {
  synthEngine.noteOff(voiceIndex);
  samplePlayer.noteOff(voiceIndex);
}

bool MusicPlayer::voiceActive(int voiceIndex) const {
  return synthEngine.voiceActive(voiceIndex) || samplePlayer.voiceActive(voiceIndex);
}

int16_t MusicPlayer::renderSample() {
  if (sequenceActive) {
    songPlayer.tick();
  }

  int16_t sample = mixer.renderSample();

  if (gainSamplesRemaining > 0u && gainSamples > 0u) {
    const uint32_t progressed = gainSamples - gainSamplesRemaining + 1u;
    gainCurrent = gainStart + ((gainTarget - gainStart) * progressed) / gainSamples;
    --gainSamplesRemaining;
    if (gainSamplesRemaining == 0u) {
      gainCurrent = gainTarget;
      if (stopPending && gainCurrent == 0) {
        finishStop();
        return 0;
      }
    }
  }

  return (sample * gainCurrent) / 255;
}

void MusicPlayer::startFade(uint8_t targetVolume, uint16_t fadeMs) {
  gainStart = gainCurrent;
  gainTarget = targetVolume;
  if (fadeMs == 0u || gainStart == gainTarget) {
    gainCurrent = gainTarget;
    gainSamples = 0u;
    gainSamplesRemaining = 0u;
    if (stopPending && gainCurrent == 0) {
      finishStop();
    }
    return;
  }
  gainSamples = msToSamples(sampleRate(), fadeMs);
  gainSamplesRemaining = gainSamples;
}

void MusicPlayer::finishStop() {
  synthEngine.reset();
  samplePlayer.reset();
  currentSong = nullptr;
  sequenceActive = false;
  stopPending = false;
  gainCurrent = steadyVolume;
  gainStart = steadyVolume;
  gainTarget = steadyVolume;
  gainSamples = 0u;
  gainSamplesRemaining = 0u;
}

}  // namespace SGFAudio
