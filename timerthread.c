#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "plugmanager.h"
#include "threadutils.h"
#include "timerthread.h"

static const unsigned int startTimeH[4] = { 5, 5, 19, 5 };
static const unsigned int startTimeM[4] = { 0, 30, 0, 0 };
static const unsigned int endTimeH[4] = { 23, 22, 23, 23 };
static const unsigned int endTimeM[4] = { 0, 30, 0, 0 };

void *timer_thread_main(void *data)
{
	int plug = (int)data;
	setupPlug(plug);
	int tid = plug + 1;
	struct timespec timestamp;
	struct tm timeStruct;
	int st, et;
	bool on;

	while(appRunning())
	{
		if(tryLockPlugStrong(plug))
		{
			if(clock_gettime(CLOCK_REALTIME, &timestamp) == -1)
			{
				fprintf(stderr, "[TIMER THREAD #%d] Error getting realtime!\n", tid);
				stopThreads();
				break;
			}

			localtime_r(&(timestamp.tv_sec), &timeStruct);
			printf("[TIMER_THREAD #%d] --- %02d.%02d.%d %02d:%02d:%02d:%09ld!\n", tid, timeStruct.tm_mday, timeStruct.tm_mon + 1, timeStruct.tm_year + 1900, timeStruct.tm_hour, timeStruct.tm_min, timeStruct.tm_sec, timestamp.tv_nsec);

			st = startTimeH[plug];
			et = endTimeH[plug];
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
				on = timeStruct.tm_hour == st;
				if(on)
					on = timeStruct.tm_min >= startTimeM[plug];
				else
				{
					on = timeStruct.tm_hour == et;
					if(on)
							on = timeStruct.tm_min < endTimeM[plug];
					else if(st < et)
						on = timeStruct.tm_hour > st && timeStruct.tm_hour < et;
					else
					{
						on = timeStruct.tm_hour > st;
						if(!on)
							on = timeStruct.tm_hour < et;
					}
				}
			}
			else
			{
				on = startTimeM[plug] == endTimeM[plug];
				if(!on)
				{
					on = startTimeM[plug] < endTimeM[plug];
					if(on)
						on = timeStruct.tm_min >= startTimeM[plug] && timeStruct.tm_min < endTimeM[plug];
					else
					{
						on = startTimeM[plug] >= timeStruct.tm_min;
						if(!on)
							on = timeStruct.tm_min < endTimeM[plug];
					}
				}
			}

			if(on)
			{
				if(!getPlugState(plug))
				{
					setPlugState(plug, true);
					printf("[TIMER_THREAD #%d] Turned on plug at %02d.%02d.%d %02d:%02d:%02d:%09ld!\n", tid, timeStruct.tm_mday, timeStruct.tm_mon + 1, timeStruct.tm_year + 1900, timeStruct.tm_hour, timeStruct.tm_min, timeStruct.tm_sec, timestamp.tv_nsec);
				}
			}
			else
			{
				if(getPlugState(plug))
				{
					setPlugState(plug, false);
					printf("[TIMER_THREAD #%d] Turned off plug at %02d.%02d.%d %02d:%02d:%02d:%09ld!\n", tid, timeStruct.tm_mday, timeStruct.tm_mon + 1, timeStruct.tm_year + 1900, timeStruct.tm_hour, timeStruct.tm_min, timeStruct.tm_sec, timestamp.tv_nsec);
				}
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

