#include "Display.h"

#include <Arduino.h>
#include <gfx4desp32_gen4_ESP32_35CT.h>

#include <algorithm>
#include <string.h>

namespace {

constexpr int RAIL_Y = 222;
constexpr int BUTTON_TOP = 238;
constexpr int ROLL_TOP = 92;
constexpr int ROLL_HEIGHT = 78;
constexpr int ROLL_TRAVEL = 62;
constexpr uint32_t FRAME_INTERVAL_MS = 25;
constexpr uint32_t TOUCH_INTERVAL_MS = 8;
constexpr uint16_t BACKGROUND = WHITE;
constexpr uint16_t PANEL = 0xEFFF;
constexpr uint16_t PRIMARY = 0x04FC;
constexpr uint16_t HIGHLIGHT = 0x867D;
constexpr uint16_t MUTED = 0x5ACB;
constexpr uint16_t TEXT = 0x4208;

constexpr uint8_t GLYPHS[][7] = {
    {14, 17, 19, 21, 25, 17, 14},
    {4, 12, 4, 4, 4, 4, 14},
    {14, 17, 1, 2, 4, 8, 31},
    {30, 1, 1, 14, 1, 1, 30},
    {2, 6, 10, 18, 31, 2, 2},
    {31, 16, 16, 30, 1, 1, 30},
    {6, 8, 16, 30, 17, 17, 14},
    {31, 1, 2, 4, 8, 8, 8},
    {14, 17, 17, 14, 17, 17, 14},
    {14, 17, 17, 15, 1, 2, 12},
    {0, 0, 4, 0, 4, 0, 0}
};

constexpr int WHEEL_CENTERS[] = {45, 101, 197, 254, 303, 352, 409};

struct Wheel {
  int oldDigit = -1;
  int digit = -1;
  int lastOffset = -1;
  uint32_t sinceMs = 0;
};

gfx4desp32_gen4_ESP32_35CT gfx;
Wheel wheels[7];
int lastRenderedState = -1;
int lastProgressDot = -1;
uint32_t lastTouchPollMs = 0;
uint32_t lastRenderMs = 0;
bool touchHeld = false;

void drawLabel(int centerX, int y, const char* text, int size) {
  gfx.TextColor(TEXT, BACKGROUND);
  gfx.TextSize(size);
  gfx.MoveTo(centerX - static_cast<int>(strlen(text)) * 4 * size, y);
  gfx.print(text);
}

void drawControl(PlayerCommand key, int x, uint16_t color) {
  constexpr int y = 275;

  switch (key) {
    case PlayerCommand::Previous:
      gfx.RectangleFilled(x - 20, y - 17, x - 14, y + 17, color);
      gfx.TriangleFilled(x - 14, y, x + 16, y - 17, x + 16, y + 17,
                         color);
      break;

    case PlayerCommand::Stop:
      gfx.RectangleFilled(x - 16, y - 16, x + 16, y + 16, color);
      break;

    case PlayerCommand::Play:
      gfx.TriangleFilled(x - 15, y - 18, x - 15, y + 18, x + 17, y,
                         color);
      break;

    case PlayerCommand::Pause:
      gfx.RectangleFilled(x - 15, y - 17, x - 5, y + 17, color);
      gfx.RectangleFilled(x + 5, y - 17, x + 15, y + 17, color);
      break;

    case PlayerCommand::Next:
      gfx.TriangleFilled(x - 16, y - 17, x - 16, y + 17, x + 14, y,
                         color);
      gfx.RectangleFilled(x + 14, y - 17, x + 20, y + 17, color);
      break;

    case PlayerCommand::Eject:
      gfx.TriangleFilled(x - 18, y + 5, x, y - 18, x + 18, y + 5, color);
      gfx.RectangleFilled(x - 18, y + 11, x + 18, y + 17, color);
      break;

    case PlayerCommand::None:
      break;
  }
}

void drawControls() {
  constexpr PlayerCommand keys[] = {
      PlayerCommand::Previous, PlayerCommand::Stop, PlayerCommand::Play,
      PlayerCommand::Pause, PlayerCommand::Next, PlayerCommand::Eject
  };

  for (int i = 0; i < 6; ++i) {
    const uint16_t color =
        Communication::enabled(keys[i]) ? PRIMARY : MUTED;
    drawControl(keys[i], 40 + 80 * i, color);
  }
}

void drawDigit(int center, int y, int digit) {
  for (int row = 0; row < 7; ++row) {
    const int top = std::max(ROLL_TOP + 2, y + row * 6);
    const int bottom = std::min(ROLL_TOP + ROLL_HEIGHT - 3, y + row * 6 + 5);
    if (top > bottom) {
      continue;
    }

    for (int column = 0; column < 5; ++column) {
      if (GLYPHS[digit][row] & (16 >> column)) {
        gfx.RectangleFilled(center - 15 + column * 6, top,
                            center - 10 + column * 6, bottom, PRIMARY);
      }
    }
  }
}

void drawWheelFrame(int left, int right) {
  gfx.Line(left + 6, ROLL_TOP, right + 6, ROLL_TOP, PRIMARY);
  gfx.Line(left + 6, ROLL_TOP + ROLL_HEIGHT,
           right + 6, ROLL_TOP + ROLL_HEIGHT, PRIMARY);

  const int edges[] = {left, right};
  for (int x : edges) {
    int lastX = x + 6;
    for (int y = 1; y <= ROLL_HEIGHT; ++y) {
      const int dy = y - ROLL_HEIGHT / 2;
      const int edgeX =
          x + 6 * dy * dy / (ROLL_HEIGHT * ROLL_HEIGHT / 4);
      gfx.Line(lastX, ROLL_TOP + y - 1, edgeX, ROLL_TOP + y, PRIMARY);
      lastX = edgeX;
    }
  }
}

void drawWheel(int index, int value, uint32_t nowMs) {
  Wheel& wheel = wheels[index];
  if (value != wheel.digit) {
    wheel.oldDigit = wheel.digit;
    wheel.digit = value;
    wheel.sinceMs = nowMs;
    wheel.lastOffset = -1;
  }

  const uint32_t ageMs = static_cast<uint32_t>(nowMs - wheel.sinceMs);
  const int offset =
      wheel.oldDigit < 0 || ageMs >= 200
          ? ROLL_TRAVEL
          : static_cast<int>(ageMs * ROLL_TRAVEL / 200);
  if (wheel.lastOffset == offset) {
    return;
  }
  wheel.lastOffset = offset;

  const int center = WHEEL_CENTERS[index];
  const int y = ROLL_TOP + (ROLL_HEIGHT - 42) / 2;
  gfx.RectangleFilled(center - 15, ROLL_TOP + 2, center + 14,
                      ROLL_TOP + ROLL_HEIGHT - 3, PANEL);
  if (offset < ROLL_TRAVEL) {
    drawDigit(center, y + offset, wheel.oldDigit);
  }
  drawDigit(center, y + offset - ROLL_TRAVEL, wheel.digit);
}

void drawProgress(uint32_t elapsedMs, uint16_t trackLengthSeconds) {
  const int dot = 20 + static_cast<uint64_t>(elapsedMs) * 439 /
                            (1000UL * trackLengthSeconds);
  if (dot == lastProgressDot) {
    return;
  }

  if (lastProgressDot >= 0) {
    gfx.RectangleFilled(lastProgressDot - 6, RAIL_Y - 6,
                        lastProgressDot + 6, RAIL_Y + 6, BACKGROUND);
  }

  gfx.RectangleFilled(20, RAIL_Y - 1, 459, RAIL_Y + 1, MUTED);
  gfx.RectangleFilled(20, RAIL_Y - 1, dot, RAIL_Y + 1, PRIMARY);
  gfx.CircleFilled(dot, RAIL_Y, 6, HIGHLIGHT);
  lastProgressDot = dot;
}

void resetWheels() {
  for (Wheel& wheel : wheels) {
    wheel = Wheel{};
  }
}

}

namespace Display {

void begin() {
  gfx.begin();
  gfx.Orientation(LANDSCAPE);
  gfx.ScrollEnable(false);
  gfx.BacklightOn(true);
  gfx.Font(FONT2);
#if !CD_AUTODEMO
  gfx.touch_Set(TOUCH_ENABLE);
#endif
}

void render(uint32_t nowMs) {
  const PlayerState& player = Communication::state();
  const int state = static_cast<int>(player.mode);
  const bool changed = state != lastRenderedState;
  const bool status = player.mode == PlayerMode::Reading ||
                      player.mode == PlayerMode::Eject;
  const bool toc = player.mode == PlayerMode::Toc;
  const bool timed = player.mode == PlayerMode::Play ||
                     player.mode == PlayerMode::Pause;

  if (changed) {
    const bool hadWheels =
        lastRenderedState >= 0 &&
        lastRenderedState != static_cast<int>(PlayerMode::Reading) &&
        lastRenderedState != static_cast<int>(PlayerMode::Eject);
    if (status || !hadWheels) {
      gfx.Cls(BACKGROUND);
      resetWheels();
    } else {
      gfx.RectangleFilled(0, 0, 479, ROLL_TOP - 1, BACKGROUND);
    }

    gfx.RectangleFilled(16, RAIL_Y - 7, 463, RAIL_Y + 7, BACKGROUND);
    gfx.RectangleFilled(20, RAIL_Y - 1, 459, RAIL_Y + 1,
                        timed ? MUTED : PRIMARY);
    lastProgressDot = -1;
    drawControls();

    if (status) {
      drawLabel(240, 80,
                player.mode == PlayerMode::Eject ? "EJECT" : "READING", 4);
    } else {
      drawLabel(74, 45, toc ? "Trackanzahl" : "Track", toc ? 1 : 2);
      drawLabel(305, 45, toc ? "Gesamtlaufzeit"
                             : timed ? "Spielzeit" : "Spieldauer", 2);
      if (player.mode == PlayerMode::Pause) {
        drawLabel(305, 12, "PAUSE", 1);
      }
      if (!hadWheels) {
        drawWheelFrame(14, 70);
        drawWheelFrame(70, 126);
        constexpr int edges[] = {165, 222, 279, 321, 378, 435};
        for (int i = 0; i < 5; ++i) {
          drawWheelFrame(edges[i], edges[i + 1]);
        }
      }
    }
    lastRenderedState = state;
  }

  if (status) {
    return;
  }

  const int track = toc ? Communication::trackCount() : player.track + 1;
  const uint32_t seconds =
      toc ? Communication::discLengthSeconds()
          : timed ? player.elapsedMs / 1000
                  : Communication::trackLengthSeconds(player.track);
  const int digits[] = {
      track / 10,
      track % 10,
      static_cast<int>(seconds / 600),
      static_cast<int>((seconds / 60) % 10),
      10,
      static_cast<int>((seconds % 60) / 10),
      static_cast<int>(seconds % 10)
  };

  for (int i = 0; i < 7; ++i) {
    drawWheel(i, digits[i], nowMs);
  }

  if (timed) {
    drawProgress(player.elapsedMs,
                 Communication::trackLengthSeconds(player.track));
  }
}

PlayerCommand pollTouch(uint32_t nowMs) {
#if CD_AUTODEMO
  (void)nowMs;
  return PlayerCommand::None;
#else
  if (static_cast<uint32_t>(nowMs - lastTouchPollMs) < TOUCH_INTERVAL_MS) {
    return PlayerCommand::None;
  }
  lastTouchPollMs = nowMs;

  gfx.touch_Update();
  const int pen = gfx.touch_GetPen();
  if (pen == TOUCH_PRESSED && !touchHeld) {
    touchHeld = true;
    const int x = gfx.touch_GetX();
    const int y = gfx.touch_GetY();
    if (x >= 0 && x < 480 && y >= BUTTON_TOP && y < 320) {
      return static_cast<PlayerCommand>(x / 80);
    }
  } else if (pen == TOUCH_RELEASED || pen == NOTOUCH) {
    touchHeld = false;
  }

  return PlayerCommand::None;
#endif
}

PlayerCommand demoInput(uint32_t nowMs) {
#if CD_AUTODEMO
  constexpr PlayerCommand steps[] = {
      PlayerCommand::Next, PlayerCommand::Play, PlayerCommand::Pause,
      PlayerCommand::Play, PlayerCommand::Next, PlayerCommand::Stop,
      PlayerCommand::Eject, PlayerCommand::Eject
  };
  constexpr uint32_t STEP_INTERVAL_MS = 4500;
  static uint8_t step = 0;
  static uint32_t lastStepMs = Communication::state().readingSinceMs;

  if (static_cast<uint32_t>(nowMs - lastStepMs) < STEP_INTERVAL_MS) {
    return PlayerCommand::None;
  }
  lastStepMs = nowMs;
  const PlayerCommand command = steps[step];
  step = static_cast<uint8_t>((step + 1) %
                              (sizeof(steps) / sizeof(steps[0])));
  return command;
#else
  (void)nowMs;
  return PlayerCommand::None;
#endif
}

bool shouldRender(uint32_t nowMs) {
  if (static_cast<uint32_t>(nowMs - lastRenderMs) < FRAME_INTERVAL_MS) {
    return false;
  }
  lastRenderMs = nowMs;
  return true;
}

}
