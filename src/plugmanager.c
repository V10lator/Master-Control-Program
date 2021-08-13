#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "minimal_gpio.h"

#include "config.h"
#include "plugmanager.h"

#define GPIO_PLUG1	17
#define GPIO_PLUG2	18
#define GPIO_PLUG3	22
#define GPIO_PLUG4	23

volatile static atomic_bool plugLocked[4] = { false, false, false, false };
volatile static bool plugState [4];

static const unsigned int gpios[4] = { GPIO_PLUG1, GPIO_PLUG2, GPIO_PLUG3, GPIO_PLUG4 };

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

bool setPlugDaily(const int plug, const unsigned int startTimeH, const unsigned int startTimeM, const unsigned int endTimeH, const unsigned int endTimeM)
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
	st = startTimeH;
	et = endTimeH;
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
			on = timeStruct.tm_min >= startTimeM;
		else
		{
			on = timeStruct.tm_hour == et;
			if(on)
					on = timeStruct.tm_min < endTimeM;
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
		on = startTimeM == endTimeM;
		if(!on)
		{
			on = startTimeM < endTimeM;
			if(on)
				on = timeStruct.tm_min >= startTimeM && timeStruct.tm_min < endTimeM;
			else
			{
				on = startTimeM >= timeStruct.tm_min;
				if(!on)
					on = timeStruct.tm_min < endTimeM;
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

bool setPlugInterval(int plug, CONFIG_MODE mode)
{
	// TODO
	return true;
}

bool setPlugOnce(int plug, CONFIG_MODE mode)
{
	// TODO
	return true;
}

bool setPlugTimed(int plug)
{
	CONFIG_MODE mode = configGetMode(plug);
	switch(mode.id)
	{
		case PROG_MODE_DAILY:
			return setPlugDaily(plug, mode.var1, mode.var2, mode.var3, mode.var4);
		case PROG_MODE_ONCE:
			return setPlugOnce(plug, mode);
		case PROG_MODE_INTERVAL:
			return setPlugInterval(plug, mode);
		default:
			return false;
	}
}
