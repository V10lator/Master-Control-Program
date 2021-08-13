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

static volatile CONFIG *conf = NULL;
static volatile bool changed = false;
const CONFIG defConf = {
				.version = 0,	// Version
				.mode[0] = {	// Config mode array
					.id = PROG_MODE_INTERVAL,
					.var1 = 15,	// Minutes till on
					.var2 = 15,	// Minutes till off
					.var3 = 0,
					.var4 = 0,
				},
				.mode[1] = {
					.id = PROG_MODE_INTERVAL,
					.var1 = 15,
					.var2 = 15,
					.var3 = 0,
					.var4 = 0,
				},
				.mode[2] = {
					.id = PROG_MODE_DAILY,
					.var1 = 18,	// Start time hour
					.var2 = 30,	// Start time minute
					.var3 = 23,	// End time hour
					.var4 = 0,	// End time minute
				},
				.mode[3] = {
					.id = PROG_MODE_DAILY,
					.var1 = 5,
					.var2 = 0,
					.var3 = 23,
					.var4 = 0,
				},
			};

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

		conf->version = defConf.version;
		conf->mode[0] = defConf.mode[0];
		conf->mode[1] = defConf.mode[1];
		conf->mode[2] = defConf.mode[2];
		conf->mode[3] = defConf.mode[3];

		changed = true;
		bool ret = saveConfig();
		if(!ret)
		{
			free((void *)conf);
			conf = NULL;
		}

		return ret;
	}

	uint8_t *buf = (uint8_t *)malloc(sizeof(CONFIG));
	if(buf == NULL)
	{
		fprintf(stderr, "[CONFIG MANAGER] OUT OF MEMORY!\n");
		fclose(f);
		return false;
	}

	fread(buf, 1, sizeof(unsigned int), f);
	unsigned int version = *(unsigned int *)buf;
	if(version > defConf.version)
	{
		fprintf(stderr, "[CONFIG MANAGER] Config version %d higher than maximum supported (%d)\n", version, defConf.version);
		fclose(f);
		deinitConfig();
		return false;
	}

	buf += sizeof(unsigned int);

	fread(buf, 1, sizeof(CONFIG) - sizeof(unsigned int), f);
	fclose(f);
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
		fclose(f);
		return false;
	}

	changed = false;
	fflush(f);
	fclose(f);
	return true;
}

CONFIG_MODE configGetMode(unsigned int index)
{
	return conf->mode[index];
}
