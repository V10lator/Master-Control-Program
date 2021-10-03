#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "mcp23017.h"

#include "buttonhandler.h"
#include "progMode/auto.h"
#include "progMode/manual.h"

#define BUTTON_PM_TIMESET	0x0040
#define BUTTON_PM_MANUAL	0x0020
#define BUTTON_PM_AUTO		0x0010
#define BUTTON_PM_ONCE		0x0008
#define BUTTON_PM_WEEKLY1	0x0004
#define BUTTON_PM_WEEKLY2	0x0002

static uint16_t progMode = 0x0000;
static bool oldButton[10];
static const uint16_t btns[10] = { BUTTON_ONOFF, BUTTON_DAY_M, BUTTON_DAY_P, BUTTON_HOUR_M, BUTTON_HOUR_P, BUTTON_MIN_M, BUTTON_MIN_P, BUTTON_CHN_M, BUTTON_CHN_P, BUTTON_CANCEL };
// Ordered by chance of being selected:
static const uint16_t btns_pm[6] = { BUTTON_PM_AUTO, BUTTON_PM_MANUAL, BUTTON_PM_TIMESET, BUTTON_PM_ONCE, BUTTON_PM_WEEKLY1, BUTTON_PM_WEEKLY2 };

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
static inline void modeWeekly(bool first, uint16_t buttons)
{
	internalHandleButtons(buttons);
}

void handleButtons()
{
	uint16_t buttons = read_mcp23017();

	// Check proram mode switch for invalid settings and reset it if needed
	if(buttons & BUTTON_PM_TIMESET && buttons & BUTTON_PM_WEEKLY2)
	{
		// MP23017 error, try to reset
		fprintf(stderr, "[BUTTON HANDLER] MCP23017 error detected!\n");
		close_mcp23017();
		initialize_mcp23017(); //TODO: Error handling
		return;
	}

	// Read current program mode switch state
	for(int i = 0; i < 6; i++)
	{
		if(buttons & btns_pm[i] && progMode != btns_pm[i])
		{
			printf("[BUTTON_HANDLER] Switching progam mode!\n");
			// If prog mode switched and there is an old mode (won't be at boot) - Switch away from it
			switch(progMode)
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
				default:
					break;
			}

			// If program mode changed: Switch to new mode
			// We o this outside of the previous if as we'll switch from no program mode at startup
			progMode = btns_pm[i];
			switch(progMode)
			{
				case BUTTON_PM_AUTO:
					modeSwitchToAuto();
					return;
				case BUTTON_PM_MANUAL:
					modeSwitchToManual();
					return;
				case BUTTON_PM_TIMESET:
				case BUTTON_PM_ONCE:
				case BUTTON_PM_WEEKLY1:
				case BUTTON_PM_WEEKLY2:
					return;
			}

			// No need to break the loop here as we already returned in the switch above
		}
	}

	// Real buttons
	uint16_t filteredButtons = 0x0000;
	for(int i = 0; i < 10; i++)
		if(buttons & btns[i])
		{
			if(!oldButton[i])
			{
				filteredButtons |= btns[i];
				oldButton[i] = true;
			}
		}
		else
			oldButton[i] = false;

	// Handle filterred buttons to the current program mode
	switch(progMode)
	{
		case BUTTON_PM_TIMESET:
			modeTimeset(filteredButtons);
			break;
		case BUTTON_PM_MANUAL:
			modeManual(filteredButtons);
			break;
		case BUTTON_PM_AUTO:
			modeAuto(filteredButtons);
			break;
		case BUTTON_PM_ONCE:
			modeOnce(filteredButtons);
			break;
		case BUTTON_PM_WEEKLY1:
			modeWeekly(true, filteredButtons);
			break;
		case BUTTON_PM_WEEKLY2:
			modeWeekly(false, filteredButtons);
	}
}
