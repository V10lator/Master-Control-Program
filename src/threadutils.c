#include <limits.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "threadutils.h"

#define RT_STACK_SIZE (128 * 1024) // 128 kB

typedef struct MCP_THREAD MCP_THREAD;
struct MCP_THREAD
{
	pthread_t pthread;
	MCP_THREAD *next;
};

volatile static atomic_bool lock = false;
volatile static bool __appRunning = true;
volatile static MCP_THREAD *threads = NULL;

bool appRunning()
{
	return __appRunning;
}

bool startThread(const char *name, bool realtime, void *(*function)(void *), void *data)
{
	MCP_THREAD *thread = (MCP_THREAD *)malloc(sizeof(MCP_THREAD));
	if(thread == NULL)
	{
		fprintf(stderr, "[THREAD MANAGER] OUT OF MEMORY!\n");
		return false;
	}
	thread->next = NULL;

	bool ret = false;
	while(!atomic_compare_exchange_weak(&lock, &ret, true))
	{
		ret = false;
		usleep(20);
	}

	pthread_attr_t attr;
	pthread_attr_t *attrPtr = &attr;
	if(pthread_attr_init(attrPtr))
	{
		fprintf(stderr, "[THREAD MANAGER] Error initializing thread!");
		goto threadError;
	}
	if(realtime)
	{
		struct sched_param param = {
			.sched_priority = 95,
		};
		if(pthread_attr_setschedpolicy(attrPtr, SCHED_FIFO) ||
			pthread_attr_setschedparam(attrPtr, &param) ||
			pthread_attr_setinheritsched(attrPtr, PTHREAD_EXPLICIT_SCHED) ||
			pthread_attr_setstacksize(attrPtr, RT_STACK_SIZE)
		)
		{
			fprintf(stderr, "[THREAD MANAGER] Error initializing realtime scheduling for %s!\n", name);
			goto threadError;
		}
	}

	ret = pthread_create(&(thread->pthread), attrPtr, function, data) == 0;
	if(!ret)
	{
		fprintf(stderr, "[THREAD MANAGER] Error starting %s!\n", name);
		goto threadError;
	}

	if(pthread_setname_np(thread->pthread, name) != 0)
		fprintf(stderr, "[THREAD MANAGER] Error setting name \"%s\"!\n", name);

	if(threads == NULL)
		threads = thread;
	else
	{
		volatile MCP_THREAD *cur = threads;
		while(cur->next != NULL)
			cur = cur->next;

		cur->next = thread;
	}

	goto threadExit;

threadError:
	ret = false;
	free((void *)thread);
threadExit:
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

	volatile MCP_THREAD *cur = threads;
	while(cur != NULL)
	{
		pthread_kill(cur->pthread, SIGUSR1);
		cur = cur->next;
	}

	cur = threads;
	void *tmp;
	while(cur != NULL)
	{
		pthread_join(cur->pthread, NULL);
		tmp = (void *)cur;
		cur = cur->next;
		free(tmp);
	}

	threads = NULL;
	lock = false;
}

void dummyHandler(int signal) {};
