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

void *timer_thread_main(void *data)
{
	int plug = (int)data;
	setupPlug(plug);
	signal(SIGUSR1, dummyHandler);

	struct timespec timestamp;
	long nsec;
	time_t sec;
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


		nsec = 1000000000;
		sec = 59;
		clock_gettime(CLOCK_REALTIME, &timestamp);

		nsec -= timestamp.tv_nsec;
		timestamp.tv_nsec = nsec;

		timestamp.tv_sec %= 60;
		sec -= timestamp.tv_sec;
		timestamp.tv_sec = sec;

		nanosleep(&timestamp, NULL);
	}

	pthread_exit(NULL);
}
