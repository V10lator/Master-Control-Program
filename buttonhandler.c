#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "mcp23017.h"

#include "buttonhandler.h"
#include "modeAuto.h"
#include "modeManual.h"

#define BUTTON_PM_TIMESET	0x0040
#define BUTTON_PM_MANUAL	0x0020
#define BUTTON_PM_AUTO		0x0010
#define BUTTON_PM_ONCE		0x0008
#define BUTTON_PM_WEEKLY1	0x0004
#define BUTTON_PM_WEEKLY2	0x0002

uint16_t progMode = 0x0000;
bool oldButton[10];

bool initButtonhandler()
{
	for(int i = 0; i < 10; i++)
		oldButton[i] = false;

	return initialize_mcp23017();
}

static inline void internalHandleButtons(uint16_t buttons)
{
	if(buttons & BUTTON_ONOFF)
		printf("[BUTTON HANDLER] Pressed: On/Off\n");
	if(buttons & BUTTON_DAY_M)
		printf("[BUTTON HANDLER] Pressed: Day-\n");
	if(buttons & BUTTON_DAY_P)
		printf("[BUTTON HANDLER] Pressed: Day+\n");
	if(buttons & BUTTON_HOUR_M)
		printf("[BUTTON HANDLER] Pressed: Hour-\n");
	if(buttons & BUTTON_HOUR_P)
		printf("[BUTTON HANDLER] Pressed: Hour+\n");
	if(buttons & BUTTON_MIN_M)
		printf("[BUTTON HANDLER] Pressed: Minute-\n");
	if(buttons & BUTTON_MIN_P)
		printf("[BUTTON HANDLER] Pressed: Minute+\n");
	if(buttons & BUTTON_CHN_M)
		printf("[BUTTON HANDLER] Pressed: Channel-\n");
	if(buttons & BUTTON_CHN_P)
		printf("[BUTTON HANDLER] Pressed: Channel+\n");
	if(buttons & BUTTON_CANCEL)
		printf("[BUTTON HANDLER] Pressed: Cancel\n");
}

#define modeTimeset internalHandleButtons
#define modeOnce internalHandleButtons
static void modeWeekly(bool first, uint16_t buttons)
{
	internalHandleButtons(buttons);
}

void handleButtons()
{
	uint16_t buttons = read_mcp23017();
	uint16_t tmp = 0x0000;

	// Program mode switch
	if(buttons & BUTTON_PM_TIMESET && progMode != BUTTON_PM_TIMESET)
	{
		printf("[BUTTON HANDLER] Program mode switched to: Time set\n");
		tmp = progMode;
		progMode = BUTTON_PM_TIMESET;
	}
	else if(buttons & BUTTON_PM_MANUAL && progMode != BUTTON_PM_MANUAL)
	{
		printf("[BUTTON HANDLER] Program mode switched to: Manual\n");
		tmp = progMode;
		progMode = BUTTON_PM_MANUAL;
	}
	else if(buttons & BUTTON_PM_AUTO && progMode != BUTTON_PM_AUTO)
	{
		printf("[BUTTON HANDLER] Program mode switched to: Auto\n");
		tmp = progMode;
		progMode = BUTTON_PM_AUTO;
	}
	else if(buttons & BUTTON_PM_ONCE && progMode != BUTTON_PM_ONCE)
	{
		printf("[BUTTON HANDLER] Program mode switched to: Once\n");
		tmp = progMode;
		progMode = BUTTON_PM_ONCE;
	}
	else if(buttons & BUTTON_PM_WEEKLY1 && progMode != BUTTON_PM_WEEKLY1)
	{
		printf("[BUTTON HANDLER] Program mode switched to: Weekly 1\n");
		tmp = progMode;
		progMode = BUTTON_PM_WEEKLY1;
	}
	else if(buttons & BUTTON_PM_WEEKLY2 && progMode != BUTTON_PM_WEEKLY2) {
		printf("[BUTTON HANDLER] Program mode switched to: Weekly 2\n");
		tmp = progMode;
		progMode = BUTTON_PM_WEEKLY2;
	}

	if(tmp != 0x0000)
	{
		switch(tmp)
		{
			case BUTTON_PM_AUTO:
				modeSwitchFromAuto();
				break;
			case BUTTON_PM_MANUAL:
				modeSwitchFromManual();
				break;
			case BUTTON_PM_TIMESET:
			case BUTTON_PM_ONCE:
			case BUTTON_PM_WEEKLY1:
			case BUTTON_PM_WEEKLY2:
				break;
		}

		switch(progMode)
		{
			case BUTTON_PM_AUTO:
				modeSwitchToAuto();
				break;
			case BUTTON_PM_MANUAL:
				modeSwitchToManual();
				break;
			case BUTTON_PM_TIMESET:
			case BUTTON_PM_ONCE:
			case BUTTON_PM_WEEKLY1:
			case BUTTON_PM_WEEKLY2:
				return;
		}
		return;
	}

	// Real buttons
	if(buttons & BUTTON_ONOFF)
	{
		if(!oldButton[0])
		{
			tmp |= BUTTON_ONOFF;
			oldButton[0] = true;
		}
	}
	else
		oldButton[0] = false;

	if(buttons & BUTTON_DAY_M)
	{
		if(!oldButton[1])
		{
			tmp |= BUTTON_DAY_M;
			oldButton[1] = true;
		}
	}
	else
		oldButton[1] = false;

	if(buttons & BUTTON_DAY_P)
	{
		if(!oldButton[2])
		{
			tmp |= BUTTON_DAY_P;
			oldButton[2] = true;
		}
	}
	else
		oldButton[2] = false;

	if(buttons & BUTTON_HOUR_M)
	{
		if(!oldButton[3])
		{
			tmp |= BUTTON_HOUR_M;
			oldButton[3] = true;
		}
	}
	else
		oldButton[3] = false;

	if(buttons & BUTTON_HOUR_P)
	{
		if(!oldButton[4])
		{
			tmp |= BUTTON_HOUR_P;
			oldButton[4] = true;
		}
	}
	else
		oldButton[4] = false;

	if(buttons & BUTTON_MIN_M)
	{
		if(!oldButton[5])
		{
			tmp |= BUTTON_MIN_M;
			oldButton[5] = true;
		}
	}
	else
		oldButton[5] = false;

	if(buttons & BUTTON_MIN_P)
	{
		if(!oldButton[6])
		{
			tmp |= BUTTON_MIN_P;
			oldButton[6] = true;
		}
	}
	else
		oldButton[6] = false;

	if(buttons & BUTTON_CHN_M)
	{
		if(!oldButton[7])
		{
			tmp |= BUTTON_CHN_M;
			oldButton[7] = true;
		}
	}
	else
		oldButton[7] = false;

	if(buttons & BUTTON_CHN_P)
	{
		if(!oldButton[8])
		{
			tmp |= BUTTON_CHN_P;
			oldButton[8] = true;
		}
	}
	else
		oldButton[8] = false;

	if(buttons & BUTTON_CANCEL)
	{
		if(!oldButton[9])
		{
			tmp |= BUTTON_CANCEL;
			oldButton[9] = true;
		}
	}
	else
		oldButton[9] = false;

	switch(progMode)
	{
		case BUTTON_PM_TIMESET:
			modeTimeset(tmp);
			break;
		case BUTTON_PM_MANUAL:
			modeManual(tmp);
			break;
		case BUTTON_PM_AUTO:
			modeAuto(tmp);
			break;
		case BUTTON_PM_ONCE:
			modeOnce(tmp);
			break;
		case BUTTON_PM_WEEKLY1:
			modeWeekly(true, tmp);
			break;
		case BUTTON_PM_WEEKLY2:
			modeWeekly(false, tmp);
	}
}
