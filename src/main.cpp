#include <Arduino.h>

#include "Communication.h"
#include "Display.h"

void setup() {
  Display::begin();
  const uint32_t nowMs = millis();
  Communication::begin(nowMs);
  Display::render(nowMs);
}

void loop() {
  const uint32_t nowMs = millis();

  Communication::tick(nowMs);

  const PlayerCommand command =
#if CD_AUTODEMO
      Display::demoInput(nowMs);
#else
      Display::pollTouch(nowMs);
#endif
  if (command != PlayerCommand::None) {
    Communication::command(command, nowMs);
  }

  if (Display::shouldRender(nowMs)) {
    Display::render(nowMs);
  }

  delay(1);
}
