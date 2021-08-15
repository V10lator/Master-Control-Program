#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "threadutils.h"

#define MAX_THREADS 5

volatile static atomic_bool lock = false;
volatile static bool __appRunning = true;
volatile static pthread_t threads[MAX_THREADS];
volatile static int index = -1;

bool appRunning()
{
	return __appRunning;
}

bool startThread(bool realtime, void *(*function)(void *), void *data)
{
	bool fa = false;
	while(!atomic_compare_exchange_weak(&lock, &fa, true))
	{
		fa = false;
		usleep(20);
	}

	if(++index == MAX_THREADS)
	{
		fprintf(stderr, "[THREAD MANAGER] Too many threads!\n");
		index--;
		lock = false;
		return false;
	}

	pthread_attr_t attr;
	pthread_attr_t *attrPtr;
	struct sched_param param;
	if(realtime)
	{
		param.sched_priority = 95;
		if(pthread_attr_init(&attr) || pthread_attr_setschedpolicy(&attr, SCHED_FIFO) ||
			pthread_attr_setschedparam(&attr, &param) || pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED)
		)
		{
			fprintf(stderr, "[THREAD MANAGER] Error initializing realtime scheduling!\n");
			attrPtr = NULL;
		}
		else
			attrPtr = &attr;
	}
	else
		attrPtr = NULL;

	bool ret = pthread_create(threads + index, attrPtr, function, data) == 0;
	if(!ret)
	{
		fprintf(stderr, "[THREAD MANAGER] Error starting thread!\n");
		index--;
	}

	lock = false;
	return ret;
}

void stopThreads()
{
	__appRunning = false;
	bool fa = false;
	while(!atomic_compare_exchange_weak(&lock, &fa, true))
        {
                fa = false;
                usleep(20);
        }

	for(int i = 0; i <= index; i++)
		pthread_kill(threads[i], SIGUSR1);
	for(int i = 0; i <= index; i++)
		pthread_join(threads[i], NULL);
	index = -1;
	lock = false;
}

void dummyHandler(int signal) {};
