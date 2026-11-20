#include "Display.h"
#include "CommunicationConfig.h"

#include <Arduino.h>
#include <gfx4desp32_gen4_ESP32_35CT.h>

#include <algorithm>
#include <string.h>

namespace {

constexpr int RAIL_Y = 222;
constexpr int BUTTON_TOP = 238;
constexpr int ROLL_TOP = 72;
constexpr int ROLL_HEIGHT = 90;
constexpr int ROLL_TRAVEL = 74;
constexpr uint32_t FRAME_INTERVAL_MS = 25;
constexpr uint32_t TOUCH_INTERVAL_MS = 8;
constexpr uint16_t BACKGROUND = WHITE;
constexpr uint16_t PANEL = 0x057C;
constexpr uint16_t PRIMARY = 0x057C;
constexpr uint16_t HIGHLIGHT = 0x7E9E;
constexpr uint16_t MUTED = 0xBE79;
constexpr uint16_t TEXT = 0x4A89;

// Fest eingebettete Ziffern-Bitmaps, 40 x 56 Pixel.
constexpr uint64_t GLYPHS[11][56] = {
  {
    0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0xff0000ULL, 0x3ffc000ULL,
    0x7ffe000ULL, 0xf80f800ULL, 0x1e007800ULL, 0x3c003c00ULL, 0x38001e00ULL, 0x78000e00ULL, 0x70000f00ULL,
    0x70000700ULL, 0xf0000700ULL, 0xe0000380ULL, 0xe0000380ULL, 0xe0000380ULL, 0x1e0000380ULL, 0x1e00003c0ULL,
    0x1c00001c0ULL, 0x1c00001c0ULL, 0x1c00001c0ULL, 0x1c00001c0ULL, 0x1c00001c0ULL, 0x1c00001c0ULL, 0x1c00001c0ULL,
    0x1c00001c0ULL, 0x1c00001c0ULL, 0x1c00001c0ULL, 0x1c00001c0ULL, 0x1c00001c0ULL, 0x1c00001c0ULL, 0x1c00001c0ULL,
    0x1e00001c0ULL, 0xe00003c0ULL, 0xe0000380ULL, 0xe0000380ULL, 0xe0000380ULL, 0xf0000780ULL, 0x70000700ULL,
    0x78000700ULL, 0x38000f00ULL, 0x3c001e00ULL, 0x1c001e00ULL, 0xf003c00ULL, 0xf80f800ULL, 0x7fff000ULL,
    0x1ffe000ULL, 0x7f8000ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL
  },
  {
    0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x1800000ULL, 0x1c00000ULL,
    0x1e00000ULL, 0x1f80000ULL, 0x1fc0000ULL, 0x1ff0000ULL, 0x1e78000ULL, 0x1e3e000ULL, 0x1e0f800ULL,
    0x1e03c00ULL, 0x1e00800ULL, 0x1e00000ULL, 0x1e00000ULL, 0x1e00000ULL, 0x1e00000ULL, 0x1e00000ULL,
    0x1e00000ULL, 0x1e00000ULL, 0x1e00000ULL, 0x1e00000ULL, 0x1e00000ULL, 0x1e00000ULL, 0x1e00000ULL,
    0x1e00000ULL, 0x1e00000ULL, 0x1e00000ULL, 0x1e00000ULL, 0x1e00000ULL, 0x1e00000ULL, 0x1e00000ULL,
    0x1e00000ULL, 0x1e00000ULL, 0x1e00000ULL, 0x1e00000ULL, 0x1e00000ULL, 0x1e00000ULL, 0x1e00000ULL,
    0x1e00000ULL, 0x1e00000ULL, 0x1e00000ULL, 0x1e00000ULL, 0x1e00000ULL, 0x1e00000ULL, 0x1e00000ULL,
    0x1e00000ULL, 0x1e00000ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL
  },
  {
    0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0xff8000ULL, 0x3fff000ULL,
    0x7fffc00ULL, 0x1f807e00ULL, 0x1e000f00ULL, 0x3c000700ULL, 0x38000100ULL, 0x78000000ULL, 0x70000000ULL,
    0x70000000ULL, 0x70000000ULL, 0x70000000ULL, 0x70000000ULL, 0x70000000ULL, 0x70000000ULL, 0x70000000ULL,
    0x78000000ULL, 0x38000000ULL, 0x3c000000ULL, 0x1c000000ULL, 0xe000000ULL, 0xf000000ULL, 0x7800000ULL,
    0x3c00000ULL, 0x1f00000ULL, 0x780000ULL, 0x3e0000ULL, 0x1f0000ULL, 0x78000ULL, 0x3e000ULL,
    0x1f000ULL, 0x7800ULL, 0x3c00ULL, 0x1e00ULL, 0xf00ULL, 0x700ULL, 0x780ULL,
    0x380ULL, 0x3c0ULL, 0x1c0ULL, 0x1c0ULL, 0x1c0ULL, 0x7fffffc0ULL, 0x7fffffc0ULL,
    0x7fffffc0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL
  },
  {
    0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x3fe000ULL, 0xfffc00ULL,
    0x3ffff00ULL, 0x7e01f00ULL, 0xf800300ULL, 0xf000000ULL, 0x1e000000ULL, 0x1e000000ULL, 0x1c000000ULL,
    0x1c000000ULL, 0x1c000000ULL, 0x1c000000ULL, 0x1c000000ULL, 0x1c000000ULL, 0x1c000000ULL, 0xe000000ULL,
    0xe000000ULL, 0x7000000ULL, 0x7800000ULL, 0x3e00000ULL, 0xfc0000ULL, 0x3ff800ULL, 0x7ff800ULL,
    0x1fff800ULL, 0x7f00000ULL, 0xf800000ULL, 0x1e000000ULL, 0x1c000000ULL, 0x3c000000ULL, 0x38000000ULL,
    0x78000000ULL, 0x78000000ULL, 0x70000000ULL, 0x70000000ULL, 0x70000000ULL, 0x78000000ULL, 0x38000000ULL,
    0x38000000ULL, 0x3c000000ULL, 0x1c000000ULL, 0x1e000000ULL, 0xf8001c0ULL, 0x7e00fc0ULL, 0x1ffff80ULL,
    0xffff00ULL, 0x1ff800ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL
  },
  {
    0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x1e000000ULL, 0x1e000000ULL,
    0x1f000000ULL, 0x1f000000ULL, 0x1f800000ULL, 0x1f800000ULL, 0x1dc00000ULL, 0x1dc00000ULL, 0x1ce00000ULL,
    0x1c700000ULL, 0x1c700000ULL, 0x1c380000ULL, 0x1c3c0000ULL, 0x1c1c0000ULL, 0x1c0e0000ULL, 0x1c0f0000ULL,
    0x1c070000ULL, 0x1c038000ULL, 0x1c03c000ULL, 0x1c01e000ULL, 0x1c00e000ULL, 0x1c007000ULL, 0x1c007800ULL,
    0x1c003c00ULL, 0x1c001c00ULL, 0x1c000e00ULL, 0x1c000f00ULL, 0x1c000780ULL, 0x1c0003c0ULL, 0x1c0001c0ULL,
    0x1c0000e0ULL, 0xffffffff0ULL, 0xffffffff0ULL, 0xfffffffe0ULL, 0x1c000000ULL, 0x1c000000ULL, 0x1c000000ULL,
    0x1c000000ULL, 0x1c000000ULL, 0x1c000000ULL, 0x1c000000ULL, 0x1c000000ULL, 0x1c000000ULL, 0x1c000000ULL,
    0x1c000000ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL
  },
  {
    0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x1ffff800ULL, 0x1ffffc00ULL,
    0x1ffffc00ULL, 0x1c00ULL, 0x1c00ULL, 0x1c00ULL, 0x1c00ULL, 0x1c00ULL, 0x1c00ULL,
    0x1c00ULL, 0x1c00ULL, 0x1c00ULL, 0x1c00ULL, 0x1c00ULL, 0x1c00ULL, 0x1c00ULL,
    0xc00ULL, 0xe00ULL, 0x3ffe00ULL, 0x1fffe00ULL, 0x3fffe00ULL, 0xfe00000ULL, 0xf000000ULL,
    0x1e000000ULL, 0x3c000000ULL, 0x38000000ULL, 0x78000000ULL, 0x78000000ULL, 0x70000000ULL, 0x70000000ULL,
    0x70000000ULL, 0x70000000ULL, 0x70000000ULL, 0x70000000ULL, 0x70000000ULL, 0x70000000ULL, 0x78000000ULL,
    0x38000000ULL, 0x3c000000ULL, 0x1c000000ULL, 0x1e000000ULL, 0xf800100ULL, 0x7e00f00ULL, 0x3ffff00ULL,
    0xffff00ULL, 0x1ff800ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL
  },
  {
    0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x1ff00000ULL, 0x7ffe0000ULL,
    0x7fff0000ULL, 0x600fc000ULL, 0x3e000ULL, 0x1f000ULL, 0xf000ULL, 0x7800ULL, 0x3c00ULL,
    0x1c00ULL, 0x1e00ULL, 0xe00ULL, 0xe00ULL, 0xf00ULL, 0x700ULL, 0x700ULL,
    0x700ULL, 0x1fe0780ULL, 0x7ff8380ULL, 0xfffe380ULL, 0x3f01f380ULL, 0x3c007380ULL, 0x78003b80ULL,
    0xf0001f80ULL, 0xe0000f80ULL, 0x1e0000f80ULL, 0x1e0000780ULL, 0x1c0000780ULL, 0x1c0000380ULL, 0x1c0000380ULL,
    0x1c0000380ULL, 0x1c0000380ULL, 0x1c0000380ULL, 0x1c0000780ULL, 0x1c0000700ULL, 0x1c0000700ULL, 0x1e0000700ULL,
    0xe0000e00ULL, 0xf0000e00ULL, 0x70001e00ULL, 0x78003c00ULL, 0x3c007800ULL, 0x1f01f000ULL, 0xfffe000ULL,
    0x3ffc000ULL, 0xff0000ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL
  },
  {
    0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x3ffffffc0ULL, 0x3ffffffc0ULL,
    0x1ffffffc0ULL, 0x1e0000000ULL, 0xe0000000ULL, 0x70000000ULL, 0x70000000ULL, 0x38000000ULL, 0x38000000ULL,
    0x1c000000ULL, 0x1c000000ULL, 0xe000000ULL, 0xe000000ULL, 0x7000000ULL, 0x7000000ULL, 0x3800000ULL,
    0x3800000ULL, 0x1c00000ULL, 0x1c00000ULL, 0xe00000ULL, 0xe00000ULL, 0xf00000ULL, 0x700000ULL,
    0x780000ULL, 0x380000ULL, 0x380000ULL, 0x3c0000ULL, 0x1c0000ULL, 0x1e0000ULL, 0xe0000ULL,
    0xe0000ULL, 0xf0000ULL, 0x70000ULL, 0x70000ULL, 0x78000ULL, 0x78000ULL, 0x38000ULL,
    0x38000ULL, 0x3c000ULL, 0x3c000ULL, 0x1c000ULL, 0x1c000ULL, 0x1c000ULL, 0x1e000ULL,
    0x1e000ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL
  },
  {
    0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x7f0000ULL, 0x3ffe000ULL,
    0x7fff000ULL, 0xf80f800ULL, 0x1e003c00ULL, 0x3c001e00ULL, 0x38000e00ULL, 0x78000f00ULL, 0x70000700ULL,
    0x70000700ULL, 0x70000700ULL, 0x70000700ULL, 0x70000700ULL, 0x70000700ULL, 0x78000f00ULL, 0x38000e00ULL,
    0x3c001e00ULL, 0x1e003c00ULL, 0xf007800ULL, 0x7c1f000ULL, 0x3ffe000ULL, 0x1ffc000ULL, 0x7fff000ULL,
    0x1f80f800ULL, 0x3e003e00ULL, 0x3c000e00ULL, 0x78000f00ULL, 0xf0000780ULL, 0xe0000380ULL, 0xe0000380ULL,
    0x1e00003c0ULL, 0x1e00003c0ULL, 0x1e00001c0ULL, 0x1e00001c0ULL, 0x1e00003c0ULL, 0x1e00003c0ULL, 0xe0000380ULL,
    0xe0000380ULL, 0xf0000780ULL, 0x70000700ULL, 0x78000f00ULL, 0x3c001e00ULL, 0x1f007c00ULL, 0xffff800ULL,
    0x3ffe000ULL, 0xff8000ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL
  },
  {
    0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0xff0000ULL, 0x3ffe000ULL,
    0x7fff000ULL, 0xf80f800ULL, 0x1e003c00ULL, 0x3c001e00ULL, 0x38000f00ULL, 0x78000700ULL, 0x70000780ULL,
    0x70000380ULL, 0xe0000380ULL, 0xe00003c0ULL, 0xe00001c0ULL, 0xe00001c0ULL, 0xe00001c0ULL, 0x1e00001c0ULL,
    0x1e00003c0ULL, 0x1e00003c0ULL, 0x1e0000380ULL, 0x1f0000380ULL, 0x1f0000780ULL, 0x1f8000f00ULL, 0x1dc000f00ULL,
    0x1ce001e00ULL, 0x1e7807c00ULL, 0x1e3fff800ULL, 0xe1fff000ULL, 0xe07f8000ULL, 0xe0000000ULL, 0xe0000000ULL,
    0xe0000000ULL, 0xf0000000ULL, 0x70000000ULL, 0x70000000ULL, 0x78000000ULL, 0x38000000ULL, 0x38000000ULL,
    0x1c000000ULL, 0x1e000000ULL, 0xf000000ULL, 0x7800000ULL, 0x3c00000ULL, 0x1f00700ULL, 0xffff00ULL,
    0x3fff00ULL, 0xffc00ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL
  },
  {
    0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL,
    0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x380000ULL, 0x7c0000ULL, 0x7c0000ULL,
    0x3c0000ULL, 0x380000ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL,
    0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL,
    0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL,
    0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x380000ULL, 0x7c0000ULL,
    0x7c0000ULL, 0x3c0000ULL, 0x380000ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL,
    0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL
  }
};

constexpr int WHEEL_CENTERS[] = {54, 102, 189, 251, 297, 344, 406};

// Gleiche Reihenfolge fuer Darstellung und Touch-Auswertung.
constexpr PlayerCommand CONTROL_KEYS[] = {
    PlayerCommand::Loop, PlayerCommand::Previous, PlayerCommand::Pause, PlayerCommand::Play,
    PlayerCommand::Stop, PlayerCommand::Next, PlayerCommand::Eject
};

struct Wheel {
  int oldDigit = -1;
  int digit = -1;
  int lastOffset = -1;
  uint32_t sinceMs = 0;
};

gfx4desp32_gen4_ESP32_35CT gfx;
Wheel wheels[7];
int lastRenderedState = -1;
int lastControlMask = -1;
RepeatMode lastRenderedRepeat = RepeatMode::Off;
const char* lastNotice = nullptr;
int lastProgressDot = -1;
uint32_t lastTouchPollMs = 0;
uint32_t lastRenderMs = 0;
bool touchHeld = false;
PlayerCommand heldKey = PlayerCommand::None;
uint32_t heldSince = 0;
bool longSent = false;

// Gefuelltes abgerundetes Feld aus vorhandenen Bibliotheksprimitiven.
void roundedPanel(int left, int top, int right, int bottom, int radius,
                  uint16_t color) {
  gfx.RectangleFilled(left + radius, top, right - radius, bottom, color);
  gfx.RectangleFilled(left, top + radius, right, bottom - radius, color);
  gfx.CircleFilled(left + radius, top + radius, radius, color);
  gfx.CircleFilled(right - radius, top + radius, radius, color);
  gfx.CircleFilled(left + radius, bottom - radius, radius, color);
  gfx.CircleFilled(right - radius, bottom - radius, radius, color);
}

void drawLabel(int centerX, int y, const char* text, int size,
               uint16_t background = BACKGROUND) {
  gfx.TextColor(TEXT, background);
  gfx.TextSize(size);
  gfx.MoveTo(centerX - gfx.strWidth(text) / 2, y);
  gfx.print(text);
}

void drawBadge(int centerX, const char* text) {
  constexpr int top = 10;
  constexpr int radius = 16;
  int size = 2;
  gfx.TextSize(size);
  // Links stehen 156 Pixel zur Verfuegung, rechts 280.
  const int maxWidth = centerX < 158 ? 156 : 280;
  if (gfx.strWidth(text) + 2 * (radius + 4) > maxWidth) {
    size = 1;
    gfx.TextSize(size);
  }
  const int textHeight = gfx.charHeight('M');
  const int width = gfx.strWidth(text) + 2 * (radius + 4);
  const int height = std::max(36, textHeight + 16);
  roundedPanel(centerX - width / 2, top, centerX + width / 2,
               top + height, radius, HIGHLIGHT);
  drawLabel(centerX, top + (height - textHeight) / 2,
            text, size, HIGHLIGHT);
}

void drawNumberPanels() {
  roundedPanel(30, ROLL_TOP, 126, ROLL_TOP + ROLL_HEIGHT, 14, PANEL);
  roundedPanel(158, ROLL_TOP, 437, ROLL_TOP + ROLL_HEIGHT, 14, PANEL);
  gfx.Line(78, ROLL_TOP, 78, ROLL_TOP + ROLL_HEIGHT, HIGHLIGHT);
  const int separators[] = {220, 282, 313, 375};
  for (int x : separators)
    gfx.Line(x, ROLL_TOP, x, ROLL_TOP + ROLL_HEIGHT, HIGHLIGHT);
}

void drawControl(PlayerCommand key, int x, uint16_t color) {
  constexpr int y = 275;

  switch (key) {
    case PlayerCommand::Loop:
      // Zwei gegenlaeufige Pfeile wie in der Designvorlage.
      gfx.Line(x - 18, y + 5, x - 18, y - 9, color);
      gfx.RectangleFilled(x - 20, y - 9, x - 15, y + 5, color);
      gfx.RectangleFilled(x - 17, y - 14, x + 9, y - 9, color);
      gfx.TriangleFilled(x + 8, y - 20, x + 20, y - 11,
                         x + 8, y - 3, color);
      gfx.RectangleFilled(x + 15, y - 5, x + 20, y + 9, color);
      gfx.RectangleFilled(x - 9, y + 9, x + 17, y + 14, color);
      gfx.TriangleFilled(x - 8, y + 3, x - 20, y + 11,
                         x - 8, y + 20, color);
      break;

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

    default:
      break;
  }
}

void drawControls() {
  gfx.RectangleFilled(0, BUTTON_TOP, 479, 319, BACKGROUND);
  if (Communication::state().loopEnabled)
    roundedPanel(7, 245, 61, 318, 12, HIGHLIGHT);
  for (int i = 0; i < 7; ++i) {
    const uint16_t color =
        Communication::enabled(CONTROL_KEYS[i]) ? PRIMARY : MUTED;
    drawControl(CONTROL_KEYS[i], (2 * i + 1) * 480 / 14, color);
  }
  if (Communication::state().loopEnabled)
    drawLabel(34, 300,
              Communication::state().repeat == RepeatMode::Track ? "1" : "CD",
              1, HIGHLIGHT);
}

void drawDigit(int center, int y, int digit) {
  for (int row = 0; row < 56; ++row) {
    const int screenY = y + row;
    if (screenY < ROLL_TOP + 2 || screenY > ROLL_TOP + ROLL_HEIGHT - 3)
      continue;
    // 11 ist ein Platzhalter fuer noch nicht empfangene Werte.
    const uint64_t bits = digit == 11
        ? ((row >= 26 && row <= 29) ? (0xFFFFULL << 12) : 0)
        : GLYPHS[digit][row];
    int column = 0;
    while (column < 40) {
      if ((bits & (1ULL << column)) == 0) {
        ++column;
        continue;
      }
      const int start = column;
      while (column < 40 && (bits & (1ULL << column))) ++column;
      gfx.Line(center - 20 + start, screenY, center - 21 + column,
               screenY, WHITE);
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
  const int y = ROLL_TOP + (ROLL_HEIGHT - 56) / 2;
  const int clearHalfWidth = index == 4 ? 4 : 20;
  gfx.RectangleFilled(center - clearHalfWidth, ROLL_TOP + 2,
                      center + clearHalfWidth - 1,
                      ROLL_TOP + ROLL_HEIGHT - 3, PANEL);
  if (offset < ROLL_TRAVEL) {
    drawDigit(center, y + offset, wheel.oldDigit);
  }
  drawDigit(center, y + offset - ROLL_TRAVEL, wheel.digit);
}

void drawProgress(uint32_t elapsedMs, uint16_t trackLengthSeconds) {
  if (!trackLengthSeconds) return;
  elapsedMs = std::min(elapsedMs, uint32_t(trackLengthSeconds) * 1000U);
  const int dot = 30 + static_cast<uint64_t>(elapsedMs) * 408 /
                            (1000UL * trackLengthSeconds);
  if (dot == lastProgressDot) {
    return;
  }

  if (lastProgressDot >= 0) {
    gfx.RectangleFilled(lastProgressDot - 6, RAIL_Y - 6,
                        lastProgressDot + 6, RAIL_Y + 6, BACKGROUND);
  }

  gfx.Line(30, RAIL_Y, 438, RAIL_Y, HIGHLIGHT);
  gfx.Line(30, RAIL_Y, dot, RAIL_Y, PRIMARY);
  gfx.CircleFilled(dot, RAIL_Y, 3, PRIMARY);
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
  gfx.touch_Set(TOUCH_ENABLE);
}

void render(uint32_t nowMs) {
  const PlayerState& player = Communication::state();
  const int state = static_cast<int>(player.mode);
  const bool changed = state != lastRenderedState;
  const bool status = player.mode == PlayerMode::Reading ||
                      player.mode == PlayerMode::Eject ||
                      player.mode == PlayerMode::Waiting ||
                      player.mode == PlayerMode::Offline ||
                      player.mode == PlayerMode::Error;
  const bool toc = player.mode == PlayerMode::Toc;
  const bool timed = player.mode == PlayerMode::Play ||
                     player.mode == PlayerMode::Pause;

  if (changed) {
    const bool hadWheels =
        lastRenderedState >= 0 &&
        lastRenderedState != static_cast<int>(PlayerMode::Reading) &&
        lastRenderedState != static_cast<int>(PlayerMode::Eject) &&
        lastRenderedState != static_cast<int>(PlayerMode::Waiting) &&
        lastRenderedState != static_cast<int>(PlayerMode::Offline) &&
        lastRenderedState != static_cast<int>(PlayerMode::Error);
    if (status || !hadWheels) {
      gfx.Cls(BACKGROUND);
      resetWheels();
    } else {
      gfx.RectangleFilled(0, 0, 479, ROLL_TOP - 1, BACKGROUND);
    }

    gfx.RectangleFilled(16, RAIL_Y - 7, 463, RAIL_Y + 7, BACKGROUND);
    gfx.Line(30, RAIL_Y, 438, RAIL_Y, HIGHLIGHT);
    lastProgressDot = -1;
    drawControls();

    if (status) {
      const char* title = "READING";
      int size = 4;
      switch (player.mode) {
        case PlayerMode::Eject: title = "NO DISC"; size = 3; break;
        case PlayerMode::Waiting: title = "WARTE AUF STEUERUNG"; size = 2; break;
        case PlayerMode::Offline: title = "VERBINDUNG UNTERBROCHEN"; size = 2; break;
        case PlayerMode::Error: title = "LAUFWERKSFEHLER"; size = 2; break;
        default: break;
      }
      drawLabel(240, 80, title, size);
    } else {
      drawBadge(78, toc ? "TRACKS" : "TRACK");
      drawBadge(298, toc ? "TOTAL" : timed ? "TIME" : "DURATION");
      if (player.mode == PlayerMode::Pause) {
        drawLabel(298, 182, "PAUSE", 1);
      } else {
        gfx.RectangleFilled(260, 182, 336, 198, BACKGROUND);
      }
      if (!hadWheels) {
        drawNumberPanels();
      }
    }
    lastRenderedState = state;
  }

  int controlMask = 0;
  for (unsigned i=0; i<7; ++i)
    if (Communication::enabled(CONTROL_KEYS[i])) controlMask |= 1 << i;
  if (changed || controlMask != lastControlMask ||
      player.repeat != lastRenderedRepeat) {
    drawControls();
    lastControlMask = controlMask;
    lastRenderedRepeat = player.repeat;
  }
  if (changed || lastNotice != Communication::notice()) {
    gfx.RectangleFilled(0, 200, 479, 216, BACKGROUND);
    drawLabel(240, 200, Communication::notice(), 1);
    lastNotice = Communication::notice();
  }

  if (status) {
    return;
  }

  const int track = toc ? Communication::trackCount() : player.track + 1;
  const uint32_t seconds =
      toc ? Communication::discLengthSeconds()
          : timed ? player.elapsedMs / 1000
                  : Communication::trackLengthSeconds(player.track);
  int digits[] = {
      track / 10,
      track % 10,
      static_cast<int>(seconds / 600),
      static_cast<int>((seconds / 60) % 10),
      10,
      static_cast<int>((seconds % 60) / 10),
      static_cast<int>(seconds % 10)
  };

  if (!(toc ? player.countKnown : player.trackKnown))
    digits[0] = digits[1] = 11;
  const bool validTime = toc ? player.discKnown
                            : timed ? player.elapsedKnown : player.durationKnown;
  if (!validTime || seconds > 5999)
    digits[2] = digits[3] = digits[5] = digits[6] = 11;
  for (int i = 0; i < 7; ++i) {
    drawWheel(i, digits[i], nowMs);
  }

  if (timed && player.elapsedKnown && player.durationKnown) {
    drawProgress(player.elapsedMs,
                 Communication::trackLengthSeconds(player.track));
  } else if (lastProgressDot >= 0) {
    gfx.RectangleFilled(16, RAIL_Y - 7, 463, RAIL_Y + 7, BACKGROUND);
    gfx.Line(30, RAIL_Y, 438, RAIL_Y, HIGHLIGHT);
    lastProgressDot = -1;
  }
}

PlayerCommand pollTouch(uint32_t nowMs) {
  if (static_cast<uint32_t>(nowMs - lastTouchPollMs) < TOUCH_INTERVAL_MS)
    return PlayerCommand::None;
  lastTouchPollMs = nowMs;

  gfx.touch_Update();
  const int pen = gfx.touch_GetPen();
  if (pen == TOUCH_PRESSED) {
    const int x = gfx.touch_GetX();
    const int y = gfx.touch_GetY();
    if (!touchHeld) {
      touchHeld = true;
      heldSince = nowMs;
      longSent = false;
      heldKey = PlayerCommand::None;
      if (x >= 0 && x < 480 && y >= BUTTON_TOP && y < 320)
        heldKey = CONTROL_KEYS[x * 7 / 480];
      // Navigation und Repeat: kurz/long erst unterscheiden.
      if (heldKey == PlayerCommand::Next ||
          heldKey == PlayerCommand::Previous ||
          heldKey == PlayerCommand::Loop) return PlayerCommand::None;
      return heldKey;
    }
    if (!longSent && uint32_t(nowMs - heldSince) >= CD_LONG_PRESS_MS) {
      PlayerCommand longKey = PlayerCommand::None;
      if (heldKey == PlayerCommand::Next) longKey = PlayerCommand::SearchFwdStart;
      if (heldKey == PlayerCommand::Previous) longKey = PlayerCommand::SearchBwdStart;
      if (heldKey == PlayerCommand::Loop) longKey = PlayerCommand::Shuffle;
      if (longKey != PlayerCommand::None && Communication::enabled(longKey)) {
        longSent = true;
        return longKey;
      }
    }
  } else if (touchHeld && (pen == TOUCH_RELEASED || pen == NOTOUCH)) {
    touchHeld = false;
    const PlayerCommand key = heldKey;
    heldKey = PlayerCommand::None;
    if (longSent) {
      longSent = false;
      if (key == PlayerCommand::Next || key == PlayerCommand::Previous)
        return PlayerCommand::SearchStop;
      return PlayerCommand::None;
    }
    if (key == PlayerCommand::Next || key == PlayerCommand::Previous ||
        key == PlayerCommand::Loop) return key;
  }
  return PlayerCommand::None;
}

bool shouldRender(uint32_t nowMs) {
  if (static_cast<uint32_t>(nowMs - lastRenderMs) < FRAME_INTERVAL_MS) {
    return false;
  }
  lastRenderMs = nowMs;
  return true;
}

}
