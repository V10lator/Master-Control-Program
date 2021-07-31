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
	if(plugState[plug])
	{
		gpioWrite(gpios[plug],GPIO_LOW);
		plugState[plug] = false;
		printf("Switched %d to LOW\n", plug + 1);
		return;
	}
	gpioWrite(gpios[plug], GPIO_HIGH);
	plugState[plug] = true;
	printf("Switched %d to HIGH\n", plug + 1);
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
	for(int i = 0; i < 4; i++)
	{
		startTime[i] = i == 0 ? (6-2) : (i == 1 ? (7-2) : (i == 2 ? (20-2) : (6-2)));
		endTime[i] = (i == 1 ? 23 : 24) - 2;
	}

	printf("[TIMER THREAD] Started!\n");
	while(appRunning())
	{
		if(clock_gettime(CLOCK_TAI, &curTick) == -1)
		{
			fprintf(stderr, "[TIMER THREAD] Error getting realtime!\n");
			continue;
		}

		hour = (curTick.tv_sec % SECS_IN_DAY) / 3600;

		for(int i = 0; i < 4; i++)
		{
			if(!atomic_compare_exchange_strong(plugLocked + i, fap, true))
			{
				fa = false;
				continue;
			}

			if(hour >= startTime[i] && hour < endTime[i])
			{
				if(!plugState[i])
				{
					gpioWrite(gpios[i], GPIO_HIGH);

					unsigned int sec = curTick.tv_sec % SECS_IN_MIN;
					unsigned int min = (curTick.tv_sec % SECS_IN_HOUR) / 60;
					unsigned int thour = hour + 2;
					if(thour > 23)
						thour -=24;

					printf("[TIMER_THREAD] Turned on plug #%d at %02d:%02d:%02d:%09d!\n", i + 1, thour, min, sec, curTick.tv_nsec);
					plugState[i] = true;
				}
			}
			else
			{
				if(plugState[i])
				{
					gpioWrite(gpios[i], GPIO_LOW);

					unsigned int sec = curTick.tv_sec % SECS_IN_MIN;
					unsigned int min = (curTick.tv_sec % SECS_IN_HOUR) / 60;
					unsigned int thour = hour + 2;
					if(thour > 23)
						thour -=24;

					printf("[TIMER_THREAD] Turned off plug #%d at %02d:%02d:%02d:%09d!\n", i + 1, thour, min, sec, curTick.tv_nsec);
					plugState[i] = false;
				}
			}

			unlockPlug(i);
		}

		if(clock_gettime(CLOCK_TAI, &lastTick) == -1)
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
