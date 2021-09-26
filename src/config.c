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
#define AT24_MAGIC 0x1337BABE
#define AT24_CUR_VER 1

typedef struct __attribute__((__packed__))
{
	uint32_t magicA;
	uint32_t version;
	CONFIG_MODE mode[4];
	uint32_t magicB;
} AT24_CONFIG;

static volatile AT24_CONFIG *at24_config = NULL;
static volatile AT24_CONFIG *at24_orig = NULL;

static const CONFIG_MODE defConf[4] = {
				{
					.id = PROG_MODE_ALWAYS_ON,
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

	uint8_t *tc = (uint8_t *)malloc(fs.st_size);
	if(tc == NULL)
	{
		fprintf(stderr, "[AT24 DRIVER] OUT OF MEMORY!\n");
		goto cleanup_fd;
	}

	size_t got = 0;
	size_t got_now;
	do
	{
		got_now = read(fd, (void *)(tc + got), fs.st_size - got);
		if(got_now == -1)
		{
			fprintf(stderr, "[AT24 DRIVER] Error reading EEPROM!\n");
			goto cleanup_mem;
		}
		got += got_now;
	} while(got != fs.st_size);

	close(fd);

	AT24_CONFIG *nc = malloc(sizeof(AT24_CONFIG));
	if(nc == NULL)
	{
		fprintf(stderr, "[AT24 DRIVER] OUT OF MEMORY!\n");
		goto cleanup_mem;
	}

	memcpy((void *)nc, (void *)tc, sizeof(AT24_CONFIG));
	at24_config = nc;
	at24_orig = (AT24_CONFIG *)tc;

	if(at24_config->magicA != AT24_MAGIC)
	{
		fprintf(stderr, "[AT24 DRIVER] Magic not found, setting defaults.\n");
		at24_config->magicA = at24_config->magicB = AT24_MAGIC;
		at24_config->version = AT24_CUR_VER;
		for(int i = 0; i < 4; i++)
			at24_config->mode[i] = defConf[i];

		return saveConfig();
	}

	if(at24_config->magicB != AT24_MAGIC)
	{
		fprintf(stderr, "[AT24 DRIVER] Possibly corrupted EEPROM! Forcing save!.\n");
		at24_config->magicB = AT24_MAGIC;
		return saveConfig();
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
	if(at24_config == NULL || at24_orig == NULL)
		return false;

	int fd = open(AT24_PATH, O_WRONLY | O_SYNC);
	if(fd < 0)
	{
		fprintf(stderr, "[AT24 DRIVER] Error opening sys file!\n");
		return false;
	}

	size_t written_now;
	size_t toWrite;
	size_t dirtStart = -1;
	uint8_t *obuf = (uint8_t *)at24_orig;
	uint8_t *buf = (uint8_t *)at24_config;
	uint8_t *ptr;
	bool ret = true;
	for(size_t i = 0; i < sizeof(AT24_CONFIG); i++)
	{
		if(obuf + i == buf + i)
		{
			if(dirtStart == -1)
				continue;

			lseek(fd, dirtStart, SEEK_SET);
			toWrite = i - dirtStart;
			ptr = buf + dirtStart;
			while(toWrite)
			{
				written_now = write(fd, ptr, toWrite);
				if(written_now == -1)
				{
					fprintf(stderr, "[AT24 DRIVER] Error writing config: %s\n", strerror(errno));
					ret = false;
					goto exit_saving;
				}
				ptr += written_now;
				toWrite -= written_now;
			}
			dirtStart = -1;
		}

		if(dirtStart == -1)
			dirtStart = i;
	}
	if(dirtStart != -1)
	{
		lseek(fd, dirtStart, SEEK_SET);
		toWrite = sizeof(AT24_CONFIG);
		ptr = buf + dirtStart;
		while(toWrite)
		{
			written_now = write(fd, ptr, toWrite);
			if(written_now == -1)
			{
				fprintf(stderr, "[AT24 DRIVER] Error writing config: %s\n", strerror(errno));
				ret = false;
				goto exit_saving;
			}
			ptr += written_now;
			toWrite -= written_now;
		}
	}

exit_saving:
	close(fd);
	return ret;
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
	if(at24_orig != NULL)
	{
		void *c = (void *)at24_orig;
		at24_orig = NULL;
		free(c);
	}
}

CONFIG_MODE configGetMode(unsigned int index)
{
	return at24_config->mode[index];
}
