#pragma once
#include <stdint.h>

enum class PlayerMode : uint8_t {
  Reading, Toc, Select, Play, Pause, Eject, Waiting, Offline, Error
};
enum class PlayerCommand : uint8_t {
  Previous, Stop, Play, Pause, Next, Eject, Loop,
  SearchFwdStart, SearchBwdStart, SearchStop, Shuffle, None = 255
};
enum class RepeatMode : uint8_t { Off, Track, Disc };

struct PlayerState {
  PlayerMode mode = PlayerMode::Waiting;
  uint8_t track = 0; // intern nullbasiert; UART TRACK beginnt bei 1
  uint32_t elapsedMs = 0;
  uint32_t readingSinceMs = 0;
  bool loopEnabled = false;
  RepeatMode repeat = RepeatMode::Off;
  bool trackKnown = false;
  bool countKnown = false;
  bool elapsedKnown = false;
  bool durationKnown = false;
  bool discKnown = false;
  uint32_t trackRemainingSeconds = 0;
  uint32_t discElapsedSeconds = 0;
  uint32_t discRemainingSeconds = 0;
  bool trackRemainingKnown = false;
  bool discElapsedKnown = false;
  bool discRemainingKnown = false;
  bool searching = false;
};

namespace Communication {
void begin(uint32_t nowMs);
void tick(uint32_t nowMs);
void command(PlayerCommand key, uint32_t nowMs);
bool enabled(PlayerCommand key);
bool selectTrack(uint8_t trackNumber, uint32_t nowMs);
uint8_t trackCount();
uint16_t trackLengthSeconds(uint8_t track);
uint32_t discLengthSeconds();
const PlayerState& state();
const char* notice();
}
