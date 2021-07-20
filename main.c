#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "minimal_gpio.h"

#include "buttonhandler.h"
#include "timerthread.h"

#define SLICE_TIME 62500000 // 16 slices / sec
//#define SLICE_TIME 125000000 // 8 slices / sec

int main()
{
	printf("[MASTER CONTROL PROGRAM]\n");
	if(!gpioInitialise() || !initButtonhandler())
		return 1;

	pthread_t timer_thread;
	int timer_thread_id = pthread_create(&timer_thread, NULL, timer_thread_main, NULL);

	struct timespec startTime;
	struct timespec endTime;
	struct timespec now;

	while(true)
	{
		if(clock_gettime(CLOCK_MONOTONIC_RAW, &startTime) != 0)
		{
			fprintf(stderr, "[MAIN] Error getting time!\n");
			continue;
		}

		handleButtons();

		if(clock_gettime(CLOCK_MONOTONIC_RAW, &endTime) != 0)
		{
			fprintf(stderr, "[MAIN] Error getting time!\n");
			continue;
		}

		endTime.tv_sec -= startTime.tv_sec;
		if(endTime.tv_sec != 0)
			continue;

		endTime.tv_nsec -= startTime.tv_nsec;
		if(endTime.tv_nsec >= SLICE_TIME)
			continue;

		endTime.tv_nsec = SLICE_TIME - endTime.tv_nsec;
		nanosleep(&endTime, &startTime);
	}

	return 0;
}
