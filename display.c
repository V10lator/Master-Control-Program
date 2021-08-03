#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#include "display.h"

bool displayInitialized = false;
bool displayTimeEnabled = false;
DISPLAY_CHANNEL displayChannel = DISPLAY_CHANNEL_INVALID;
bool displayOn = false;

void updateDisplay()
{
	if(!displayInitialized)
		return;

	if(displayTimeEnabled)
	{
		//TODO: Enable time grid

		struct timespec timestamp;
		struct tm timeStruct;
		if(clock_gettime(CLOCK_REALTIME, &timestamp) == -1)
		{
			fprintf(stderr, "[DISPLAY_MANAGER] Error getting realtime!\n");
			//TODO: Show N/A or something like that
		       }
		else
		{
			localtime_r(&(timestamp.tv_sec), &timeStruct);
			//TODO: Show the time
		}

		//TODO: STUB
	}
	else
	{
		//TODO: STUB
		// Disable time grid
	}

	if(displayChannel != DISPLAY_CHANNEL_INVALID)
	{
		//TODO: StUB
	}
	else
	{
		//TODO: STUB
		// Disable channel grid
	}

	if(displayOn)
	{
		//TODO: STUB
	}
	else
	{
		//TODO: STUB
	}
}

bool initDisplay()
{
	if(displayInitialized)
		return true;

	printf("[DISPLAY MANAGER] Init: STUB!\n");
	displayInitialized = true;
	return true;
}

void disableDisplay()
{
	if(!displayInitialized)
		return;

	printf("[DISPLAY MANAGER] Disable: STUB!\n");
	displayInitialized = false;
}

void displayEnableTime(bool enable)
{
	if(!displayInitialized)
		return;

	displayTimeEnabled = enable;
	printf("[DISPLAY MANAGER] Enable time (%s): STUB!\n", enable ? "TRUE" : "FALSE");
}

void displayShowTime(int hour, int minute, int second)
{
	if(!displayInitialized)
		return;

	printf("[DISPLAY MANAGER] Show time: STUB!\n");
}

void displaySetChannel(DISPLAY_CHANNEL channel)
{
	if(!displayInitialized)
		return;

	displayChannel = channel;
//	printf("[DISPLAY MANAGER] Set channel (to %01d): STUB!\n", channel);
}

void displaySetOn(bool on)
{
	if(!displayInitialized)
		return;

	displayOn = on;
	printf("[DISPLAY MANAGER] Set on (%s): STUB!\n", on ? "TRUE" : "FALSE");
}
