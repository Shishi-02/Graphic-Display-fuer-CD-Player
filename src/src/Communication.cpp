#include "Communication.h"

namespace {

constexpr uint16_t TRACK_LENGTHS[] = {
    202, 147, 138, 171, 125, 163, 142, 154, 139,
    146, 157, 128, 166, 135, 149, 142, 148
};
constexpr uint8_t TRACK_COUNT =
    sizeof(TRACK_LENGTHS) / sizeof(TRACK_LENGTHS[0]);
constexpr uint32_t READING_DURATION_MS = 1500;

PlayerState player = {};
uint32_t previousTickMs = 0;

}

namespace Communication {

void begin(uint32_t nowMs) {
  player.mode = PlayerMode::Reading;
  player.track = 0;
  player.elapsedMs = 0;
  player.readingSinceMs = nowMs;
  previousTickMs = nowMs;
}

bool enabled(PlayerCommand key) {
  if (key == PlayerCommand::None) {
    return false;
  }
  if (key == PlayerCommand::Eject) {
    return true;
  }
  if (player.mode == PlayerMode::Reading || player.mode == PlayerMode::Eject) {
    return false;
  }
  if (key == PlayerCommand::Pause) {
    return player.mode == PlayerMode::Play;
  }
  if (key == PlayerCommand::Play) {
    return player.mode != PlayerMode::Play;
  }
  return true;
}

void command(PlayerCommand key, uint32_t nowMs) {
  if (!enabled(key)) {
    return;
  }

  switch (key) {
    case PlayerCommand::Eject:
      player.mode = player.mode == PlayerMode::Eject
                        ? PlayerMode::Reading
                        : PlayerMode::Eject;
      player.readingSinceMs = nowMs;
      player.track = 0;
      player.elapsedMs = 0;
      break;

    case PlayerCommand::Previous:
    case PlayerCommand::Next:
      if (player.mode == PlayerMode::Toc) {
        player.track = key == PlayerCommand::Next
                           ? 0
                           : static_cast<uint8_t>(TRACK_COUNT - 1);
      } else {
        player.track =
            static_cast<uint8_t>((player.track +
                                  (key == PlayerCommand::Next
                                       ? 1
                                       : TRACK_COUNT - 1)) %
                                 TRACK_COUNT);
      }
      player.elapsedMs = 0;
      if (player.mode != PlayerMode::Play) {
        player.mode = PlayerMode::Select;
      }
      break;

    case PlayerCommand::Stop:
      player.mode = PlayerMode::Toc;
      player.track = 0;
      player.elapsedMs = 0;
      break;

    case PlayerCommand::Pause:
      player.mode = PlayerMode::Pause;
      break;

    case PlayerCommand::Play:
      player.mode = PlayerMode::Play;
      break;

    case PlayerCommand::None:
      break;
  }
}

void tick(uint32_t nowMs) {
  const uint32_t deltaMs =
      static_cast<uint32_t>(nowMs - previousTickMs);
  previousTickMs = nowMs;

  if (player.mode == PlayerMode::Reading &&
      static_cast<uint32_t>(nowMs - player.readingSinceMs) >=
          READING_DURATION_MS) {
    player.mode = PlayerMode::Toc;
  }

  if (player.mode != PlayerMode::Play) {
    return;
  }

  player.elapsedMs += deltaMs;
  while (player.elapsedMs >=
         static_cast<uint32_t>(TRACK_LENGTHS[player.track]) * 1000UL) {
    player.elapsedMs -=
        static_cast<uint32_t>(TRACK_LENGTHS[player.track]) * 1000UL;
    if (++player.track == TRACK_COUNT) {
      command(PlayerCommand::Stop, nowMs);
      break;
    }
  }
}

uint8_t trackCount() {
  return TRACK_COUNT;
}

uint16_t trackLengthSeconds(uint8_t track) {
  if (track >= TRACK_COUNT) {
    return 0;
  }
  return TRACK_LENGTHS[track];
}

uint32_t discLengthSeconds() {
  uint32_t total = 0;
  for (uint8_t i = 0; i < TRACK_COUNT; ++i) {
    total += TRACK_LENGTHS[i];
  }
  return total;
}

const PlayerState& state() {
  return player;
}

}
