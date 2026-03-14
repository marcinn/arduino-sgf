#include "Song.h"

namespace SGFAudio {

SongPlayer::SongPlayer(SynthEngine& synth, const Song& song) {
  bind(synth, song);
}

void SongPlayer::bind(SynthEngine& synth, const Song& song) {
  synthEngine = &synth;
  songRef = &song;
  trackCount = 0u;

  if (song.lanes == nullptr || song.laneCount == 0u) {
    return;
  }

  const uint8_t limit = (song.laneCount > SynthEngine::MAX_VOICES)
                          ? static_cast<uint8_t>(SynthEngine::MAX_VOICES)
                          : song.laneCount;
  for (uint8_t i = 0u; i < limit; ++i) {
    const SongLane& lane = song.lanes[i];
    if (lane.instrument == nullptr || lane.pattern == nullptr || lane.voiceIndex < 0) {
      continue;
    }
    tracks[trackCount].bind(synth, lane.voiceIndex, *lane.instrument, *lane.pattern);
    ++trackCount;
  }
}

void SongPlayer::reset() {
  for (uint8_t i = 0u; i < trackCount; ++i) {
    tracks[i].reset();
  }
}

void SongPlayer::tick() {
  if (synthEngine == nullptr || songRef == nullptr) {
    return;
  }
  for (uint8_t i = 0u; i < trackCount; ++i) {
    tracks[i].tick();
  }
}

}  // namespace SGFAudio
