#pragma once

#include <stdint.h>

#include "Communication.h"

namespace Display {

void begin();
void render(uint32_t nowMs);
PlayerCommand pollTouch(uint32_t nowMs);
bool shouldRender(uint32_t nowMs);

}
