#pragma once

#include <stdbool.h>
#include <stdint.h>

/* gpio modes. */
typedef enum
{
	GPIO_MODE_INPUT		= 0,
	GPIO_MODE_OUTPUT	= 1,
	GPIO_MODE_ALT0		= 4,
	GPIO_MODE_ALT1		= 5,
	GPIO_MODE_ALT2		= 6,
	GPIO_MODE_ALT3		= 7,
	GPIO_MODE_ALT4		= 3,
	GPIO_MODE_ALT5		= 2,
} GPIO_MODE;

bool gpioInitialise(void);
void shutdownGpio();
void gpioSetBank1(uint32_t bits);
void gpioSetBank2(uint32_t bits);
void gpioClearBank1(uint32_t bits);
void gpioClearBank2(uint32_t bits);
uint32_t gpioReadBank1(void);
uint32_t gpioReadBank2(void);
void gpioTrigger(unsigned gpio, unsigned pulseLen, bool level);
void gpioWrite(unsigned gpio, bool level);
bool gpioRead(unsigned gpio);
GPIO_MODE gpioGetMode(unsigned gpio);
void gpioSetMode(unsigned gpio, GPIO_MODE mode);
