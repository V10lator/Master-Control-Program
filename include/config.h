#pragma once

typedef enum
{
	PROG_MODE_DAILY,
	PROG_MODE_ONCE,
	PROG_MODE_INTERVAL,
	PROG_MODE_ALWAYS_OFF,
	PROG_MODE_ALWAYS_ON,
} PROG_MODE;

typedef struct
{
	PROG_MODE id;
	unsigned int var1;
	unsigned int var2;
	unsigned int var3;
	unsigned int var4;
} CONFIG_MODE;

bool initConfig();
void deinitConfig();

bool saveConfig();

CONFIG_MODE configGetMode(unsigned int index);
