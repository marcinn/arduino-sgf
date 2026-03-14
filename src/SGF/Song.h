#pragma once

#include <stdint.h>

#include "Pattern.h"

namespace SGFAudio {

struct SongLane {
  int voiceIndex = -1;
  const Instrument* instrument = nullptr;
  const struct SongClip* clips = nullptr;
  uint16_t clipCount = 0;
};

struct SongClip {
  const Pattern* pattern = nullptr;
  uint16_t repeats = 1;
};

struct Song {
  const SongLane* lanes = nullptr;
  uint8_t laneCount = 0;
};

class SongPlayer {
public:
  SongPlayer() = default;
  SongPlayer(SynthEngine& synth, const Song& song);

  void bind(SynthEngine& synth, const Song& song);
  void reset();
  void tick();

private:
  struct LaneState {
    PatternTrack track{};
    const SongLane* lane = nullptr;
    uint16_t clipIndex = 0u;
    uint16_t repeatIndex = 0u;
    bool active = false;
  };

  void advanceLane(LaneState& state);

  SynthEngine* synthEngine = nullptr;
  const Song* songRef = nullptr;
  LaneState laneStates[SynthEngine::MAX_VOICES]{};
  uint8_t trackCount = 0u;
};

}  // namespace SGFAudio
