#include <limits.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/mman.h>

#include "mcp23017.h"
#include "minimal_gpio.h"

#include "buttonhandler.h"
#include "display.h"
#include "threadutils.h"
#include "timerthread.h"

#define SLICE_TIME 62500000 // 16 slices / sec
//#define SLICE_TIME 125000000 // 8 slices / sec

static pthread_t timer_thread[4];
static volatile atomic_bool exiting = false;

static inline void cleanup()
{
	close_mcp23017();
	disableDisplay();
}

static void signalHandler(int signal)
{
	bool fa = false;
        if(atomic_compare_exchange_strong(&exiting, &fa, true))
	{
		stopThreads();
		printf("[MASTER CONTROL PROGRAM] Disabling!\n");
		cleanup();
		for(int i = 0; i < 4; i++)
			pthread_join(timer_thread[i], NULL);
	}
}

int main()
{
	printf("[MASTER CONTROL PROGRAM]\n");

	if(mlockall(MCL_CURRENT|MCL_FUTURE))
	{
		fprintf(stderr, "[MAIN] Error locking memory: %m\n");
		return 1;
	}

	if(!gpioInitialise() || !initButtonhandler() || !initDisplay())
		return 1;

	pthread_attr_t attr;
	struct sched_param param;

	param.sched_priority = 80;

	if(pthread_attr_init(&attr) || pthread_attr_setschedpolicy(&attr, SCHED_FIFO) ||
		pthread_attr_setschedparam(&attr, &param) || pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED))
	{
		fprintf(stderr, "[MAIN] Error initializing realtime scheduling!\n");
		cleanup();
		return 1;
	}

	for(int i = 0; i < 4; i++)
		if(pthread_create(&timer_thread[i], &attr, timer_thread_main, (void *)i) != 0)
		{
			stopThreads();
			cleanup();
			return 2;
		}

	signal(SIGINT,signalHandler);
	signal(SIGTERM,signalHandler);
	signal(SIGQUIT,signalHandler);

	struct timespec startTime;
	struct timespec endTime;
	struct timespec now;

	while(appRunning())
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

	signalHandler(0);
	return 0;
}
