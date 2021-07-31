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
	for(int i = 0; i < 4; i++)
	{
		startTime[i] = i == 0 ? 6 : (i == 1 ? 7 : (i == 2 ? 20 : 6));
		endTime[i] = i == 1 ? 23 : 24;
	}

	while(appRunning())
	{
		if(clock_gettime(CLOCK_REALTIME, &curTick) == -1)
		{
			fprintf(stderr, "[TIMER THREAD] Error getting realtime!\n");
			continue;
		}

		localtime_r(&(curTick.tv_sec), &timeStruct);
		if(timeStruct.tm_isdst == 0)
			timeStruct.tm_hour++;
		else if(timeStruct.tm_hour == 0)
			timeStruct.tm_hour = 24;

		for(int i = 0; i < 4; i++)
		{
			if(!atomic_compare_exchange_strong(plugLocked + i, fap, true))
			{
				fa = false;
				continue;
			}

			if(timeStruct.tm_hour >= startTime[i] && timeStruct.tm_hour < endTime[i])
			{
				if(!getPlugState(i))
				{
					setPlugState(i, true);
					int th = timeStruct.tm_hour;
					if(timeStruct.tm_isdst == 0)
						th--;
					else if(th == 24)
						th = 0;

					printf("[TIMER_THREAD] Turned on plug #%d at %02d:%02d:%02d:%09d %02d.%02d.%d!\n", i + 1, th, timeStruct.tm_min, timeStruct.tm_sec, curTick.tv_nsec, timeStruct.tm_mday, timeStruct.tm_mon + 1, timeStruct.tm_year + 1900);
				}
			}
			else
			{
				if(getPlugState(i))
				{
					setPlugState(i, false);
					int th = timeStruct.tm_hour;
					if(timeStruct.tm_isdst == 0)
						th--;
					else if(th == 24)
						th = 0;

					printf("[TIMER_THREAD] Turned off plug #%d at %02d:%02d:%02d:%09d %02d.%02d.%d!\n", i + 1, th, timeStruct.tm_min, timeStruct.tm_sec, curTick.tv_nsec, timeStruct.tm_mday, timeStruct.tm_mon + 1, timeStruct.tm_year + 1900);
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
