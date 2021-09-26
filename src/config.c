#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "config.h"

#define AT24_PATH "/sys/class/i2c-dev/i2c-1/device/1-0057/eeprom"
#define AT24_MAGIC 0x1337BABE // TODO
#define AT24_CUR_VER 0

typedef struct // TODO: Make this __attribute__((__packed__) ?
{
	uint64_t magic;
	uint8_t version;
	CONFIG_MODE mode[4];
} AT24_CONFIG;

static volatile AT24_CONFIG *at24_config = NULL;
static volatile bool at24_changed = false;

static const CONFIG_MODE defConf[4] = {
				{
					.id = PROG_MODE_ALWAYS_OFF,
				},
				{
					.id = PROG_MODE_INTERVAL,
					.var1 = 15,	// Minutes till on
					.var2 = 15,	// Minutes till off
				},
				{
					.id = PROG_MODE_DAILY,
					.var1 = 18,	// Start time hour
					.var2 = 00,	// Start time minute
					.var3 = 23,	// End time hour
					.var4 = 0,	// End time minute
				},
				{
					.id = PROG_MODE_DAILY,
					.var1 = 5,
					.var2 = 0,
					.var3 = 23,
					.var4 = 0,
				},
			};


bool initConfig()
{
	int fd = open(AT24_PATH, O_RDONLY);
	if(fd < 0)
	{
		fprintf(stderr, "[AT24 DRIVER] Error opening sys file!\n");
		return false;
	}

	struct stat fs;
	if(fstat(fd, &fs) < 0)
	{
		fprintf(stderr, "[AT24 DRIVER] Error getting stats.\n");
		goto cleanup_fd;
	}

	if(sizeof(AT24_CONFIG) > fs.st_size)
	{
		fprintf(stderr, "[AT24 DRIVER] Sanity check: %lu > %lu\n", sizeof(AT24_CONFIG), fs.st_size);
		goto cleanup_fd;
	}

	AT24_CONFIG *tc = (AT24_CONFIG *)malloc(fs.st_size);
	if(tc == NULL)
	{
		fprintf(stderr, "[AT24 DRIVER] OUT OF MEMORY!\n");
		goto cleanup_fd;
	}
	size_t got = read(fd, (void *) tc, fs.st_size);
	if(got != fs.st_size)
	{
		fprintf(stderr, "[AT24 DRIVER] Error reading EEPROM!\n");
		goto cleanup_mem;
	}

	close(fd);
	at24_config = tc;

	if(at24_config->magic != AT24_MAGIC)
	{
		fprintf(stderr, "[AT24 DRIVER] Magic not found, setting defaults.\n");
		at24_config->magic = AT24_MAGIC;
		at24_config->version = AT24_CUR_VER;
		for(int i = 0; i < 4; i++)
			at24_config->mode[i] = defConf[i];

		at24_changed = true;
		saveConfig();
	}

	return true;

cleanup_mem:
	free((void *)tc);
cleanup_fd:
	close(fd);
	return false;
}

bool saveConfig()
{
	if(at24_config == NULL)
		return false;

	if(!at24_changed)
		return true;

	int fd = open(AT24_PATH, O_WRONLY | O_SYNC);
	if(fd < 0)
	{
		fprintf(stderr, "[AT24 DRIVER] Error opening sys file!\n");
		return false;
	}

	size_t written = 0;
	size_t written_now = 0;
	size_t toWrite = sizeof(AT24_CONFIG);
	uint8_t *buf = (uint8_t *)at24_config;
	while(toWrite)
	{
		written_now = write(fd, buf + written, toWrite);
		if(written_now == -1)
		{
			fprintf(stderr, "[AT24 DRIVER] Error writing config: %s\n", strerror(errno));
			close(fd);
			return false;
		}
		written += written_now;
		toWrite -= written_now;
	}

	at24_changed = false;
	close(fd);
	return true;
}

void deinitConfig()
{
	if(at24_config != NULL)
	{
		saveConfig();
		void *c = (void *)at24_config;
		at24_config = NULL;
		free(c);
	}
}

CONFIG_MODE configGetMode(unsigned int index)
{
	return at24_config->mode[index];
}
