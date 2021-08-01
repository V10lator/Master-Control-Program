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
		struct timespec toSleep;
		toSleep.tv_sec = 0;
		toSleep.tv_nsec = 4000000; // 4 MS
		nanosleep(&toSleep, NULL);
		modeSwitchToManual();
	}
	else
	{
		int oldChan = manualChan;
		if(buttons & BUTTON_CHN_P)
		{
			if(++manualChan == 4)
				manualChan = 0;
		}
		else if(buttons & BUTTON_CHN_M)
		{
			if(--manualChan == -1)
				manualChan = 3;
		}
		else
			return;

		unlockPlug(oldChan);
		modeSwitchToManual();
	}
}
