#pragma once

#include <stdint.h>

enum class PlayerMode : uint8_t {
  Reading,
  Toc,
  Select,
  Play,
  Pause,
  Eject
};

enum class PlayerCommand : uint8_t {
  Previous,
  Stop,
  Play,
  Pause,
  Next,
  Eject,
  None = 255
};

struct PlayerState {
  PlayerMode mode;
  uint8_t track;
  uint32_t elapsedMs;
  uint32_t readingSinceMs;
};

namespace Communication {

void begin(uint32_t nowMs);
void tick(uint32_t nowMs);
void command(PlayerCommand key, uint32_t nowMs);
bool enabled(PlayerCommand key);
uint8_t trackCount();
uint16_t trackLengthSeconds(uint8_t track);
uint32_t discLengthSeconds();
const PlayerState& state();

}
