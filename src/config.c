#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "config.h"

typedef struct
{
	unsigned int version;
	CONFIG_MODE mode[4];
} CONFIG;

#define MAX_CONFIG_SIZE sizeof(CONFIG)

unsigned int confVer = 0;
static volatile CONFIG *conf = NULL;
static volatile bool changed = false;

bool initConfig()
{
	FILE *f = fopen("/etc/master_control_program.conf", "rb");
	if(f == NULL)
	{
		conf = (CONFIG *)malloc(sizeof(CONFIG));
		if(conf == NULL)
		{
			fprintf(stderr, "[CONFIG MANAGER] OUT OF MEMORY!\n");
			return false;
		}

		// Default values:
		conf->version = confVer;
		CONFIG_MODE mode;
		mode.id = PROG_MODE_DAILY;
		mode.var1 = mode.var2 = mode.var3 = mode.var4 = 0;
		conf->mode[0] =
			conf->mode[1] =
			conf->mode[2] =
			conf->mode[3] = mode;

		changed = true;
		bool ret = saveConfig();
		if(!ret)
			free((void *)conf);

		return ret;
	}

	uint8_t *buf = (uint8_t *)malloc(sizeof(CONFIG));
	if(buf == NULL)
	{
		fprintf(stderr, "[CONFIG MANAGER] OUT OF MEMORY!\n");
		return false;
	}

	fread(buf, 1, sizeof(unsigned int), f);
	unsigned int version = *(unsigned int *)buf;
	if(version > confVer)
	{
		fprintf(stderr, "[CONFIG MANAGER] Config version %d higher than maximum supported (%d)\n", version, confVer);
		fclose(f);
		deinitConfig();
		return false;
	}

	buf += sizeof(unsigned int);

	fread(buf, 1, sizeof(CONFIG) - sizeof(unsigned int), f);
	conf = (CONFIG *)(buf - sizeof(unsigned int));
	return true;
}

void deinitConfig()
{
	if(conf == NULL)
		return;

	saveConfig();
	CONFIG *c = (CONFIG *)conf;
	conf = NULL;
	free((void *)c);
}

bool saveConfig()
{
	if(conf == NULL || !changed)
		return true;

	FILE *f = fopen("/etc/master_control_program.conf", "wb");
	if(f == NULL)
	{
		fprintf(stderr, "[CONFIG MANAGER] Error saving file!");
		return false;
	}

	if(fwrite((void *)conf, 1, sizeof(CONFIG), f) != sizeof(CONFIG))
	{
		fprintf(stderr, "[CONFIG MANAGER] Error writing to file!\n");
		return false;
	}

	changed = false;
	return true;
}

CONFIG_MODE configGetMode(unsigned int index)
{
	return conf->mode[index];
}
