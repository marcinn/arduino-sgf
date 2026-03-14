#pragma once

#include <stdint.h>

#include "Pattern.h"

namespace SGFAudio {

struct SongLane {
  int voiceIndex = -1;
  const Instrument* instrument = nullptr;
  const Pattern* pattern = nullptr;
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
  SynthEngine* synthEngine = nullptr;
  const Song* songRef = nullptr;
  PatternTrack tracks[SynthEngine::MAX_VOICES]{};
  uint8_t trackCount = 0u;
};

}  // namespace SGFAudio
