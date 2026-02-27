#pragma once

#include <stdint.h>

class IHardwareScroll {
public:
  virtual ~IHardwareScroll() = default;

  // Controller-native scroll definition for the active axis in the current rotation.
  virtual void setScrollArea(uint16_t fixedStart,
                             uint16_t scrollSpan,
                             uint16_t fixedEnd) = 0;

  // Controller-native scroll start address inside the configured scroll area.
  virtual void scrollTo(uint16_t offset) = 0;

  // True when positive logical screen movement requires decreasing controller offset.
  virtual bool scrollAxisInverted() const = 0;
};
