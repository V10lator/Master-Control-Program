#pragma once

#include <stdbool.h>

#define BUTTON_ONOFF	0x4000
#define BUTTON_DAY_M	0x2000
#define BUTTON_DAY_P	0x1000
#define BUTTON_HOUR_M	0x0800
#define BUTTON_HOUR_P	0x0400
#define BUTTON_MIN_M	0x0200
#define BUTTON_MIN_P	0x0100
#define BUTTON_CHN_M	0x8000
#define BUTTON_CHN_P	0x0001
#define BUTTON_CANCEL	0x0080

bool initButtonhandler();
void handleButtons();
