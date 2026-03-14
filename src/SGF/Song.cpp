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
    if (lane.instrument == nullptr || lane.clips == nullptr || lane.clipCount == 0u || lane.voiceIndex < 0) {
      continue;
    }
    if (lane.clips[0].pattern == nullptr) {
      continue;
    }
    LaneState& state = laneStates[trackCount];
    state.track.bind(synth, lane.voiceIndex, *lane.instrument, *lane.clips[0].pattern);
    state.lane = &lane;
    state.clipIndex = 0u;
    state.repeatIndex = 0u;
    state.active = true;
    ++trackCount;
  }
}

void SongPlayer::reset() {
  for (uint8_t i = 0u; i < trackCount; ++i) {
    LaneState& state = laneStates[i];
    state.clipIndex = 0u;
    state.repeatIndex = 0u;
    state.active = (state.lane != nullptr);
    if (state.lane != nullptr && state.lane->clipCount > 0u && state.lane->clips[0].pattern != nullptr) {
      state.track.reset();
      state.track.bindPattern(*state.lane->clips[0].pattern, false);
    } else {
      state.track.reset();
      state.active = false;
    }
  }
}

void SongPlayer::tick() {
  if (synthEngine == nullptr || songRef == nullptr) {
    return;
  }
  for (uint8_t i = 0u; i < trackCount; ++i) {
    LaneState& state = laneStates[i];
    if (!state.active) {
      continue;
    }
    state.track.tick();
    if (state.track.finished()) {
      advanceLane(state);
      if (state.active) {
        state.track.tick();
      }
    }
  }
}

void SongPlayer::advanceLane(LaneState& state) {
  if (!state.active || state.lane == nullptr || state.lane->clips == nullptr || state.lane->clipCount == 0u) {
    return;
  }

  const SongClip& currentClip = state.lane->clips[state.clipIndex];
  if ((state.repeatIndex + 1u) < currentClip.repeats) {
    ++state.repeatIndex;
  } else {
    state.repeatIndex = 0u;
    state.clipIndex = static_cast<uint16_t>((state.clipIndex + 1u) % state.lane->clipCount);
  }

  const SongClip& nextClip = state.lane->clips[state.clipIndex];
  if (nextClip.pattern == nullptr) {
    state.track.reset();
    state.active = false;
    return;
  }
  state.track.bindPattern(*nextClip.pattern, true);
}

}  // namespace SGFAudio
