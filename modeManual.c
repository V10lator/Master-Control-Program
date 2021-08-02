#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "buttonhandler.h"
#include "display.h"
#include "modeManual.h"
#include "plugmanager.h"

static int manualChan = 0;

void modeSwitchToManual()
{
	while(!tryLockPlug(manualChan))
		continue;

	displayEnableTime();
	displaySetChannel(manualChan + 1);
	displaySetOn(getPlugState(manualChan));
}

void modeSwitchFromManual()
{
	setPlugTimed(manualChan);
	unlockPlug(manualChan);
	displaySetChannel(DISPLAY_CHANNEL_INVALID);
	displaySetOn(false);
}

void modeManual(uint16_t buttons)
{
	if(buttons & BUTTON_ONOFF)
	{
		togglePlug(manualChan);
		displaySetOn(getPlugState(manualChan));
	}
	else if(buttons & BUTTON_CANCEL)
	{
		modeSwitchFromManual();
		modeSwitchToManual();
	}
	else if(!(buttons & BUTTON_CANCEL))
	{
		int tmpChan = manualChan;
		if(buttons & BUTTON_CHN_P)
		{
			if(++tmpChan == 4)
				tmpChan = 0;
		}
		else if(buttons & BUTTON_CHN_M)
		{
			if(--tmpChan == -1)
				tmpChan = 3;
		}
		else
			return;

		modeSwitchFromManual();
		manualChan = tmpChan;
		modeSwitchToManual();
	}
}
