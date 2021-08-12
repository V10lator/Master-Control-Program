#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "minimal_gpio.h"

#include "plugmanager.h"

#define GPIO_PLUG1	17
#define GPIO_PLUG2	18
#define GPIO_PLUG3	22
#define GPIO_PLUG4	23

volatile static atomic_bool plugLocked[4] = { false, false, false, false };
volatile static bool plugState [4];

static const unsigned int gpios[4] = { GPIO_PLUG1, GPIO_PLUG2, GPIO_PLUG3, GPIO_PLUG4 };

static const unsigned int startTimeH[4] = { 0, 5, 19, 5 };
static const unsigned int startTimeM[4] = { 0, 0, 0, 0 };
static const unsigned int endTimeH[4] = { 0, 23, 23, 23 };
static const unsigned int endTimeM[4] = { 0, 0, 0, 0 };


void setupPlug(int plug)
{
	gpioSetMode(gpios[plug], GPIO_MODE_OUTPUT);
	setPlugState(plug, false);
}

bool tryLockPlug(int plug)
{
	bool fa = false;
	return atomic_compare_exchange_weak(plugLocked + plug, &fa, true);
}

bool tryLockPlugStrong(int plug)
{
	bool fa = false;
	return atomic_compare_exchange_strong(plugLocked + plug, &fa, true);
}

void unlockPlug(int plug)
{
	plugLocked[plug] = false;
}

bool isPlugLocked(int plug)
{
	return plugLocked[plug];
}

void togglePlug(int plug)
{
	setPlugState(plug, !getPlugState(plug));
}

bool getPlugState(int plug)
{
	return plugState[plug];
}

void setPlugState(int plug, bool state)
{
	gpioWrite(gpios[plug], !state);
	plugState[plug] = state;
}

bool setPlugTimed(int plug)
{
	struct timespec timestamp;
	struct tm timeStruct;
	int st, et;
	bool on;

	if(clock_gettime(CLOCK_REALTIME, &timestamp) == -1)
	{
		fprintf(stderr, "[PLUG_MANAGER] Error getting realtime!\n");
		return false;
	}

	localtime_r(&(timestamp.tv_sec), &timeStruct);
	st = startTimeH[plug];
	et = endTimeH[plug];
	if(timeStruct.tm_isdst > 0)
	{
		if(++st == 24)
			st = 0;

		if(++et == 24)
			et = 0;
	}

	on = st == et;
	if(!on)
	{
		on = timeStruct.tm_hour == st;
		if(on)
			on = timeStruct.tm_min >= startTimeM[plug];
		else
		{
			on = timeStruct.tm_hour == et;
			if(on)
					on = timeStruct.tm_min < endTimeM[plug];
			else if(st < et)
				on = timeStruct.tm_hour > st && timeStruct.tm_hour < et;
			else
			{
				on = timeStruct.tm_hour > st;
				if(!on)
					on = timeStruct.tm_hour < et;
			}
		}
	}
	else
	{
		on = startTimeM[plug] == endTimeM[plug];
		if(!on)
		{
			on = startTimeM[plug] < endTimeM[plug];
			if(on)
				on = timeStruct.tm_min >= startTimeM[plug] && timeStruct.tm_min < endTimeM[plug];
			else
			{
				on = startTimeM[plug] >= timeStruct.tm_min;
				if(!on)
					on = timeStruct.tm_min < endTimeM[plug];
			}
		}
	}

	if(on)
	{
		if(!getPlugState(plug))
		{
			setPlugState(plug, true);
			printf("[PLUG MANAGER] Turned on plug #%d at %02d.%02d.%d %02d:%02d:%02d:%09ld!\n", plug + 1, timeStruct.tm_mday, timeStruct.tm_mon + 1, timeStruct.tm_year + 1900, timeStruct.tm_hour, timeStruct.tm_min, timeStruct.tm_sec, timestamp.tv_nsec);
		}
	}
	else
	{
		if(getPlugState(plug))
		{
			setPlugState(plug, false);
			printf("[PLUG_MANAGER] Turned off plug #%d at %02d.%02d.%d %02d:%02d:%02d:%09ld!\n", plug + 1, timeStruct.tm_mday, timeStruct.tm_mon + 1, timeStruct.tm_year + 1900, timeStruct.tm_hour, timeStruct.tm_min, timeStruct.tm_sec, timestamp.tv_nsec);
		}
	}

	return true;
}
