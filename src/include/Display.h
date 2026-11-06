#pragma once

#include <stdint.h>

#include "Communication.h"

#ifndef CD_AUTODEMO
#define CD_AUTODEMO 0
#endif

namespace Display {

void begin();
void render(uint32_t nowMs);
PlayerCommand pollTouch(uint32_t nowMs);
PlayerCommand demoInput(uint32_t nowMs);
bool shouldRender(uint32_t nowMs);

}
