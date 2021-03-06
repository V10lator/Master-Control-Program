#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "plugmanager.h"
#include "plugMode/interval.h"

static unsigned long long lastInterval[4] = { 0ull, 0ull, 0ull, 0ull };

bool setPlugInterval(int plug, const unsigned long long intervalOn, const unsigned long long intervalOff)
{
	struct timespec now;
	if(clock_gettime(CLOCK_MONOTONIC_RAW, &now) == -1)
	{
		fprintf(stderr, "[PLUG MANAGER INTERVAL] Error getting realtime!\n");
		return false;
	}

	if(now.tv_sec - lastInterval[plug] >= (getPlugState(plug) ? intervalOff : intervalOn) * 60ull)
	{
		togglePlug(plug);
//		printf("[PLUG MANAGER INTERVAL] Flipped plug #%d after %llu seconds (now %s)!\n", plug + 1, now.tv_sec - lastInterval[plug], getPlugState(plug) ? "on" : "off");
		lastInterval[plug] = now.tv_sec;
	}

	return true;
}

void resetPlugInterval(int plug)
{
	lastInterval[plug] = 0ull;
}
