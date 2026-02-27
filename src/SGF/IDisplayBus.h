#pragma once

#include <stddef.h>
#include <stdint.h>

class IDisplayBus {
public:
  virtual ~IDisplayBus() = default;

  virtual bool begin(uint32_t spiHz) = 0;
  virtual void setFrequency(uint32_t spiHz) = 0;
  virtual void hardwareReset() = 0;

  virtual void writeCommand(uint8_t command) = 0;
  virtual void writeData(const uint8_t* bytes, size_t size) = 0;

  virtual void beginDataWrite() = 0;
  virtual void writeDataChunk(const uint8_t* bytes, size_t size) = 0;
  virtual void endDataWrite() = 0;
  virtual bool supportsWritePixels565() const { return false; }
  virtual void writePixels565(const uint16_t* pixels, size_t count) {
    (void)pixels;
    (void)count;
  }

  virtual void setBacklight(uint8_t level) = 0;
};
