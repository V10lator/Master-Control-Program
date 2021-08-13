#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "plugmanager.h"

static inline bool setPlugDaily(const int plug, unsigned int startTimeH, const unsigned int startTimeM, unsigned int endTimeH, const unsigned int endTimeM)
{
	struct timespec timestamp;
	struct tm timeStruct;
	bool on;

	if(clock_gettime(CLOCK_REALTIME, &timestamp) == -1)
	{
		fprintf(stderr, "[PLUG MANAGER DAILY] Error getting realtime!\n");
		return false;
	}

	localtime_r(&(timestamp.tv_sec), &timeStruct);
	if(timeStruct.tm_isdst > 0)
	{
		if(++startTimeH == 24)
			startTimeH = 0;

		if(++endTimeH == 24)
			endTimeH = 0;
	}

	on = startTimeH == endTimeH;
	if(!on)
	{
		on = timeStruct.tm_hour == startTimeH;
		if(on)
			on = timeStruct.tm_min >= startTimeM;
		else
		{
			on = timeStruct.tm_hour == endTimeH;
			if(on)
					on = timeStruct.tm_min < endTimeM;
			else if(startTimeH < endTimeH)
				on = timeStruct.tm_hour > startTimeH && timeStruct.tm_hour < endTimeH;
			else
			{
				on = timeStruct.tm_hour > startTimeH;
				if(!on)
					on = timeStruct.tm_hour < endTimeH;
			}
		}
	}
	else
	{
		on = startTimeM == endTimeM;
		if(!on)
		{
			on = startTimeM < endTimeM;
			if(on)
				on = timeStruct.tm_min >= startTimeM && timeStruct.tm_min < endTimeM;
			else
			{
				on = startTimeM >= timeStruct.tm_min;
				if(!on)
					on = timeStruct.tm_min < endTimeM;
			}
		}
	}

	if(on)
	{
		if(!getPlugState(plug))
		{
			setPlugState(plug, true);
			printf("[PLUG MANAGER DAILY] Turned on plug #%d at %02d.%02d.%d %02d:%02d:%02d:%09ld!\n", plug + 1, timeStruct.tm_mday, timeStruct.tm_mon + 1, timeStruct.tm_year + 1900, timeStruct.tm_hour, timeStruct.tm_min, timeStruct.tm_sec, timestamp.tv_nsec);
		}
	}
	else
	{
		if(getPlugState(plug))
		{
			setPlugState(plug, false);
			printf("[PLUG_MANAGER DAILY] Turned off plug #%d at %02d.%02d.%d %02d:%02d:%02d:%09ld!\n", plug + 1, timeStruct.tm_mday, timeStruct.tm_mon + 1, timeStruct.tm_year + 1900, timeStruct.tm_hour, timeStruct.tm_min, timeStruct.tm_sec, timestamp.tv_nsec);
		}
	}

	return true;
}
