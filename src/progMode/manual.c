#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "buttonhandler.h"
#include "display.h"
#include "plugmanager.h"
#include "progMode/manual.h"

static int manualChan = 0;

void modeSwitchToManual()
{
	while(!tryLockPlug(manualChan))
		continue;

	displayEnableTime(true);
	displaySetChannel(manualChan + 1);
	displaySetOnOff(getPlugState(manualChan));
}

void modeSwitchFromManual()
{
	setPlugTimed(manualChan);
	unlockPlug(manualChan);
	displaySetChannel(DISPLAY_CHANNEL_INVALID);
	displayDisableOnOff();
	displayEnableTime(false);
}

void modeManual(uint16_t buttons)
{
	if(buttons & BUTTON_ONOFF)
	{
		togglePlug(manualChan);
		displaySetOnOff(getPlugState(manualChan));
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
