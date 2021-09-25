#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "minimal_gpio.h"

#include "config.h"
#include "plugmanager.h"
#include "plugMode/daily.h"
#include "plugMode/interval.h"
#include "plugMode/once.h"

#define GPIO_PLUG1	17
#define GPIO_PLUG2	18
#define GPIO_PLUG3	22
#define GPIO_PLUG4	23

volatile static atomic_bool plugLocked[4] = { false, false, false, false };

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
	return !gpioRead(gpios[plug]);
}

void setPlugState(int plug, bool state)
{
	gpioWrite(gpios[plug], !state);
}

bool setPlugTimed(int plug)
{
	CONFIG_MODE mode = configGetMode(plug);
	switch(mode.id)
	{
		case PROG_MODE_DAILY:
			return setPlugDaily(plug, mode.var1, mode.var2, mode.var3, mode.var4);
		case PROG_MODE_ONCE:
			return setPlugOnce(plug);
		case PROG_MODE_INTERVAL:
			return setPlugInterval(plug, mode.var1, mode.var2);
		case PROG_MODE_ALWAYS_ON:
			return setPlugAlways(plug, true);
		case PROG_MODE_ALWAYS_OFF:
			return setPlugAlways(plug, false);
		default:
			return false;
	}
}
