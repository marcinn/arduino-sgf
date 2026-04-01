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

uint16_t MusicPlayer::unitMsFromSong(const Song& song) {
  if (song.bpm == 0u || song.stepsPerBeat == 0u) {
    return 0u;
  }
  const uint32_t denom = song.bpm * song.stepsPerBeat;
  if (denom == 0u) {
    return 0u;
  }
  const uint32_t rounded = (60000u + (denom / 2u)) / denom;
  return rounded > 0u ? rounded : 1u;
}

void MusicPlayer::play(const Song& song, bool restart) {
  if (!restart && currentSong == &song && sequenceActive) {
    return;
  }
  currentSong = &song;
  stopPending = false;
  songPlayer.setUnitMsOverride(unitMsFromSong(song));
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

void MusicPlayer::advanceSamples(uint32_t sampleCount) {
  for (uint32_t i = 0u; i < sampleCount; ++i) {
    if (sequenceActive) {
      songPlayer.tick();
    }

    if (gainSamplesRemaining > 0u && gainSamples > 0u) {
      const uint32_t progressed = gainSamples - gainSamplesRemaining + 1u;
      gainCurrent = gainStart + ((gainTarget - gainStart) * progressed) / gainSamples;
      --gainSamplesRemaining;
      if (gainSamplesRemaining == 0u) {
        gainCurrent = gainTarget;
        if (stopPending && gainCurrent == 0) {
          finishStop();
          return;
        }
      }
    }
  }
}

int16_t MusicPlayer::renderSample() {
  if (stopPending && gainCurrent == 0) {
    return 0;
  }
  return (mixer.renderSample() * gainCurrent) / 255;
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
