#include <limits.h>
#include <malloc.h>
#include <pthread.h>
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
#include "config.h"
#include "display.h"
#include "threadutils.h"
#include "timerthread.h"
#include "webui/server.h"

#define SLICE_TIME 62500000 // 16 slices / sec
#define RESERVED_MEM_SIZE (64 * 1024 * 1024) // 64 m reserved for malloc

static volatile atomic_bool exiting = false;

static inline void cleanup()
{
	close_mcp23017();
	shutdownGpio();
	deinitConfig();
	stopThreads();
	disableDisplay();
}

static void signalHandler(int signal)
{
	bool fa = false;
        if(atomic_compare_exchange_strong(&exiting, &fa, true))
	{
		printf("[MASTER CONTROL PROGRAM] Disabling!\n");
		cleanup();
	}
}

#define reserveMem()							\
	do {								\
		uint8_t *buffer = (uint8_t *)malloc(RESERVED_MEM_SIZE);	\
		for(size_t i = 0; i < RESERVED_MEM_SIZE; i++)		\
			buffer[i] = 0;					\
		free((void *)buffer);					\
	} while(0);

int main()
{
	printf("[MASTER CONTROL PROGRAM] v" MCP_VERSION "\n");
	if(pthread_setname_np(pthread_self(), "MCP") != 0)
		fprintf(stderr, "[MAIN] Error setting thread name!\n");

	if(mlockall(MCL_CURRENT|MCL_FUTURE))
	{
		fprintf(stderr, "[MAIN] Error locking memory: %m\n");
		return 1;
	}

	mallopt(M_TRIM_THRESHOLD, -1);
	mallopt(M_MMAP_MAX, 0);
	reserveMem();

	if(!initDisplay())
		return 1;

	if(!gpioInitialise())
	{
		disableDisplay();
		return 1;
	}

	if(!initConfig())
	{
		disableDisplay();
		shutdownGpio();
		return 1;
	}

	if(!initButtonhandler())
	{
		disableDisplay();
		shutdownGpio();
		deinitConfig();
		return 1;
	}

	if(!startThread("[MCP] Web UI", false, webui_thread_main, NULL))
	{
		cleanup();
		return 2;
	}

	const char *plugThreadName[4] = { "[MCP] Plug #1", "[MCP] Plug #2", "[MCP] Plug #3", "[MCP] Plug #4" };
	for(int i = 0; i < 4; i++)
		if(!startThread(plugThreadName[i], true, timer_thread_main, (void *)i))
		{
			cleanup();
			return 3;
		}

	signal(SIGINT,signalHandler);
	signal(SIGTERM,signalHandler);
	signal(SIGQUIT,signalHandler);

	struct timespec startTime;
	struct timespec endTime;

	while(appRunning())
	{
		if(clock_gettime(CLOCK_MONOTONIC_RAW, &startTime) != 0)
		{
			fprintf(stderr, "[MAIN] Error getting time!\n");
			continue;
		}

		handleButtons();
//		updateDisplay();

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
