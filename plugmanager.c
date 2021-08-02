#include <stdatomic.h>
#include <stdbool.h>
#include <stdlib.h>

#include "minimal_gpio.h"

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
