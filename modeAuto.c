#include <stdbool.h>
#include <stdint.h>

#include "display.h"
#include "modeAuto.h"
#include "plugmanager.h"

void modeAuto(uint16_t buttons)
{
	int displayState = 0;
	for(int i = 0; i < 4; i++)
		if(getPlugState(i))
			displayState |= i + 1;

	displaySetChannel(displayState);
}

void modeSwitchToAuto()
{
	displayEnableTime(true);
}

void modeSwitchFromAuto()
{
	displaySetChannel(DISPLAY_CHANNEL_INVALID);
	displayEnableTime(false);
}
