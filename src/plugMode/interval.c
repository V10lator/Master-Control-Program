#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "plugmanager.h"
#include "plugMode/interval.h"

static volatile unsigned long long lastInterval[4] = { 0ull, 0ull, 0ull, 0ull };
static volatile bool lastOn[4] = { false, false, false, false };

bool setPlugInterval(int plug, const unsigned long long intervalOn, const unsigned long long intervalOff)
{
	struct timespec now;
	if(clock_gettime(CLOCK_TAI, &now) == -1)
	{
		fprintf(stderr, "[PLUG_MANAGER] Error getting realtime!\n");
		return false;
	}

	if(now.tv_sec - lastInterval[plug] >= (lastOn[plug] ? intervalOff : intervalOn) * 60ull)
	{
		lastOn[plug] = !lastOn[plug];

		setPlugState(plug, lastOn[plug]);
		printf("[PLUG_MANAGER] Flipped plug #%d after %llu seconds!\n", plug + 1, now.tv_sec - lastInterval[plug]);
		lastInterval[plug] = now.tv_sec;
	}

	return true;
}
