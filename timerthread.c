#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "minimal_gpio.h"

#include "threadutils.h"
#include "timerthread.h"

#define GPIO_PLUG1	17
#define GPIO_PLUG2	18
#define GPIO_PLUG3	22
#define GPIO_PLUG4	23

#define GPIO_LOW	true
#define GPIO_HIGH	false

#define SECS_IN_MIN	60
#define SECS_IN_HOUR	(60 * SECS_IN_MIN)
#define SECS_IN_DAY	(24 * SECS_IN_HOUR)

#define TIMER_FREQUENCY_NS 1000000 // 1 ms

volatile static atomic_bool plugLocked[4] = { false, false, false, false };
volatile static atomic_bool plugState [4];

static const unsigned int gpios[4] = { GPIO_PLUG1, GPIO_PLUG2, GPIO_PLUG3, GPIO_PLUG4 };

static inline void gpioSetup(unsigned int gpio)
{
	gpioSetMode(gpio, GPIO_MODE_OUTPUT);
	gpioWrite(gpio, GPIO_LOW);
}

bool tryLockPlug(int plug)
{
	bool fa = false;
	return atomic_compare_exchange_weak(plugLocked + plug, &fa, true);
}

void unlockPlug(int plug)
{
	plugLocked[plug] = false;
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
	gpioWrite(gpios[plug], state ? GPIO_HIGH : GPIO_LOW);
	plugState[plug] = state;
}

void *timer_thread_main(void *data)
{
	for(int i = 0; i < 4; i++)
	{
		gpioSetup(gpios[i]);
		plugState[i] = false;
	}

	struct timespec lastTick;
	struct timespec curTick;
	unsigned int hour;
	unsigned int startTime[4];
	unsigned int endTime[4];
	bool fa = false;
	bool *fap = &fa;
	struct tm timeStruct;
	int st;
	int et;
	bool on;
	for(int i = 0; i < 4; i++)
	{
		// Winter time!
		startTime[i] = (i == 0 || i == 3) ? 5 : (i == 1 ? 6 : 19);
		endTime[i] = i == 1 ? 22 : 23;
	}

	while(appRunning())
	{
		if(clock_gettime(CLOCK_REALTIME, &curTick) == -1)
		{
			fprintf(stderr, "[TIMER THREAD] Error getting realtime!\n");
			continue;
		}

		localtime_r(&(curTick.tv_sec), &timeStruct);

		for(int i = 0; i < 4; i++)
		{
			if(!atomic_compare_exchange_strong(plugLocked + i, fap, true))
			{
				fa = false;
				continue;
			}

			st = startTime[i];
			et = endTime[i];
			if(timeStruct.tm_isdst > 0)
			{
				if(++st == 24)
					st = 0;

				if(++et == 24)
					et = 0;
			}

			on = st == et;
			if(!on)
			{
				if(st < et)
					on = timeStruct.tm_hour >= st && timeStruct.tm_hour < et;
				else
				{
					on = timeStruct.tm_hour >= st;
					if(!on)
						on = timeStruct.tm_hour < et;
				}
			}

			if(on)
			{
				if(!getPlugState(i))
				{
					setPlugState(i, true);
					printf("[TIMER_THREAD] Turned on plug #%d at %02d:%02d:%02d:%09d %02d.%02d.%d!\n", i + 1, timeStruct.tm_hour, timeStruct.tm_min, timeStruct.tm_sec, curTick.tv_nsec, timeStruct.tm_mday, timeStruct.tm_mon + 1, timeStruct.tm_year + 1900);
				}
			}
			else
			{
				if(getPlugState(i))
				{
					setPlugState(i, false);
					printf("[TIMER_THREAD] Turned off plug #%d at %02d:%02d:%02d:%09d %02d.%02d.%d!\n", i + 1, timeStruct.tm_hour, timeStruct.tm_min, timeStruct.tm_sec, curTick.tv_nsec, timeStruct.tm_mday, timeStruct.tm_mon + 1, timeStruct.tm_year + 1900);
				}
			}

			unlockPlug(i);
		}

		if(clock_gettime(CLOCK_REALTIME, &lastTick) == -1)
               	{
                       	fprintf(stderr, "[TIMER_THREAD] Error getting time!\n");
                        continue;
                }

                lastTick.tv_sec -= curTick.tv_sec;
                if(lastTick.tv_sec != 0)
                        continue;

                lastTick.tv_nsec -= curTick.tv_nsec;
                if(lastTick.tv_nsec >= TIMER_FREQUENCY_NS)
                        continue;

                lastTick.tv_nsec = TIMER_FREQUENCY_NS - lastTick.tv_nsec;
                nanosleep(&lastTick, &curTick);
	}

	pthread_exit(NULL);
}
