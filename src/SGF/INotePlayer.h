#pragma once

#include <stdint.h>

#include "AudioTypes.h"

namespace SGFAudio {

class INotePlayer {
public:
  virtual ~INotePlayer() = default;

  virtual uint32_t sampleRate() const = 0;
  virtual void noteOn(int voiceIndex, NoteProgramRef program, float baseHz, uint8_t velocity = 255) = 0;
  virtual void noteOff(int voiceIndex) = 0;
  virtual bool voiceActive(int voiceIndex) const = 0;
};

}  // namespace SGFAudio
