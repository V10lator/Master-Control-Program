#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "plugmanager.h"
#include "threadutils.h"
#include "timerthread.h"

static void dummyHandler(int signal) {};

void *timer_thread_main(void *data)
{
	int plug = (int)data;
	setupPlug(plug);
	signal(SIGUSR1, dummyHandler);

	struct timespec timestamp;
	while(appRunning())
	{
		if(tryLockPlugStrong(plug))
		{
			if(!setPlugTimed(plug))
			{
				unlockPlug(plug);
				stopThreads();
				break;
			}
			unlockPlug(plug);
		}

		clock_gettime(CLOCK_REALTIME, &timestamp);

		timestamp.tv_nsec = 1000000000ull - timestamp.tv_nsec;
		timestamp.tv_sec = 59 - (timestamp.tv_sec % 60);
		nanosleep(&timestamp, NULL);
	}

	pthread_exit(NULL);
}
