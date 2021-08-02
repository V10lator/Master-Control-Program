#pragma once

#include <stdbool.h>

typedef enum
{
	DISPLAY_CHANNEL_INVALID	= 0,
	DISPLAY_CHANNEL_1	= 1,
	DISPLAY_CHANNEL_2	= 2,
	DISPLAY_CHANNEL_3	= 3,
	DISPLAY_CHANNEL_4	= 4,
} DISPLAY_CHANNEL;

bool initDisplay();
void disableDisplay();

void displayEnableTime(bool enable);

void displayShowTime(int hour, int minute, int second);

void displaySetChannel(DISPLAY_CHANNEL channel);

void displaySetOn(bool on);
