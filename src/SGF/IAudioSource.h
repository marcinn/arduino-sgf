#pragma once

#include <stdint.h>

namespace SGFAudio {

class IAudioSource {
public:
  virtual ~IAudioSource() = default;

  virtual uint32_t sampleRate() const = 0;
  virtual int16_t renderSample() = 0;
};

}  // namespace SGFAudio
