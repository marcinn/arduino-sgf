#pragma once

#include <stdint.h>

#include "IScreen.h"

namespace SGFHardware {

struct InputPins {
  uint8_t left = 0;
  uint8_t right = 0;
  uint8_t up = 0;
  uint8_t down = 0;
  uint8_t fire = 0;
};

struct DisplaySetup {
  uint32_t spiHz = 24000000u;
  IScreen::Rotation rotation = IScreen::Rotation::Landscape;
  uint8_t backlightLevel = 255u;
  uint16_t width = 0;
  uint16_t height = 0;
};

struct BoardMeta {
  const char* buildId = "";
  const char* mcuName = "";
  const char* panelName = "";
};

struct HardwareProfile {
  BoardMeta meta{};
  DisplaySetup display{};
  InputPins input{};
};

}  // namespace SGFHardware
