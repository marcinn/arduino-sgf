#pragma once

#include <stdint.h>

#include "Pattern.h"

namespace SGFAudio {

struct SongLane {
  int voiceIndex = -1;
  NoteProgramRef program{};
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
  uint16_t bpm = 0u;
  uint8_t stepsPerBeat = 4u;
};

class SongPlayer {
public:
  SongPlayer() = default;
  SongPlayer(const Song& song, INotePlayer& player);

  void bind(const Song& song, INotePlayer& player);
  void setUnitMsOverride(uint16_t unitMs) { unitMsOverride = unitMs; }
  void advanceSamples(uint32_t sampleCount);
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

  const Song* songRef = nullptr;
  INotePlayer* notePlayer = nullptr;
  static constexpr uint8_t MAX_LANES = 8;
  LaneState laneStates[MAX_LANES]{};
  uint8_t trackCount = 0u;
  uint16_t unitMsOverride = 0u;
};

}  // namespace SGFAudio
