#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#include "display.h"

#define DISPLAY_HEAT_PIN 0 //TODO

static bool displayInitialized = false;
static bool displayTimeEnabled = false;
static DISPLAY_CHANNEL displayChannel = DISPLAY_CHANNEL_INVALID;
static unsigned int displayOnOff = 0;

void updateDisplay()
{
	if(!displayInitialized)
		return;

	if(displayTimeEnabled)
	{
		struct timespec timestamp;
		struct tm timeStruct;
		if(clock_gettime(CLOCK_REALTIME, &timestamp) == -1)
		{
			fprintf(stderr, "[DISPLAY MANAGER] Error getting realtime!\n");
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
	}

	switch(displayChannel) // TODO
	{
		case DISPLAY_CHANNEL_INVALID:
		default:
			break;
	}

	switch(displayOnOff)
	{
		case 1:
			//TODO: Show "ON"
		case 2:
			//TODO: Show "OFF"
		default:
			//TODO: Disable on and off GPIOs
			break;
	}
}

bool initDisplay()
{
	if(displayInitialized)
		return true;

	printf("[DISPLAY MANAGER] Init: STUB!\n");

	// Enable heath wire
//	gpioSetMode(DISPLAY_HEAT_PIN, GPIO_MODE_OUTPUT);
//	gpioWrite(DISPLAY_HEAT_PIN, true);

	//TODO: Enable anode GPIOs

	displayInitialized = true;
	return true;
}

void disableDisplay()
{
	if(!displayInitialized)
		return;

	printf("[DISPLAY MANAGER] Disable: STUB!\n");

	// Disable heath wire
//	gpioWrite(DISPLAY_HEAT_PIN, false);

	//TODO: Disable anodes

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

void displaySetOnOff(bool on)
{
	if(!displayInitialized)
		return;

	displayOnOff = on ? 1 : 2;
	printf("[DISPLAY MANAGER] Set on (%s): STUB!\n", on ? "TRUE" : "FALSE");
}

void displayDisableOnOff()
{
	if(!displayInitialized)
		return;

	displayOnOff = 0;
	printf("[DISPLAY MANAGER] Disable display on/off: STUB!\n");

}
