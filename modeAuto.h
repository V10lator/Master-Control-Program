#pragma once

#include "display.h"
#include "stdint.h"

#define modeSwitchToAuto() displayEnableTime()
#define modeSwitchFromAuto() displaySetChannel(DISPLAY_CHANNEL_INVALID)

void modeAuto(uint16_t buttons);
