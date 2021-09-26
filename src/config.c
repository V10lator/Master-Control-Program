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
#define AT24_CUR_VER 0

static volatile CONFIG_MODE at24_config[4];
static volatile bool at24_magic_correct = true;
static volatile uint8_t at24_version = 0;
static volatile bool at24_changed[4] = { false, false, false, false };

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

	uint64_t magic;
	uint8_t *ptr = (uint8_t *)&magic;
	size_t got = 0;
	size_t got_now;
	do
	{
		got_now = read(fd, ptr + got, 8 - got);
		if(got_now == -1)
		{
			fprintf(stderr, "[AT24 DRIVER] Error reading EEPROM (1): %s\n", strerror(errno));
			goto cleanup_fd;
		}
		got += got_now;
	} while(got != 8);

	if(magic != AT24_MAGIC)
	{
		// TODO
		fprintf(stderr, "[AT24 DRIVER] Magic not found, setting defaults.\n");
		for(int i = 0; i < 4; i++)
		{
			at24_config[i] = defConf[i];
			at24_changed[i] = true;
		}
		at24_magic_correct = false;
		at24_version = -1;
		return saveConfig();
	}

	uint8_t version;
	if(read(fd, &version, 1) != 1)
	{
		fprintf(stderr, "[AT24 DRIVER] Error reading EEPROM (2): %s\n", strerror(errno));
		goto cleanup_fd;
	}

	// TODO: Versionscheck
	at24_version = version;

	for(int i = 0; i < 4; i++)
	{
		ptr = (uint8_t *)&(at24_config[i]);
		got = 0;
		do
		{
			got_now = read(fd, ptr + got, sizeof(CONFIG_MODE) - got);
			if(got_now == -1)
			{
				fprintf(stderr, "[AT24 DRIVER] Error reading EEPROM (3/%i): %s\n", i, strerror(errno));
				goto cleanup_fd;
			}
			got += got_now;
		} while(got != sizeof(CONFIG_MODE));
	}

	close(fd);
	return true;

cleanup_fd:
	close(fd);
	return false;
}

bool saveConfig()
{
	int fd = open(AT24_PATH, O_WRONLY | O_SYNC);
	if(fd < 0)
	{
		fprintf(stderr, "[AT24 DRIVER] Error opening sys file!\n");
		return false;
	}

	size_t toWrite, written, written_now;

	if(!at24_magic_correct)
	{
		toWrite = 8;
		written = 0;
		uint64_t buf = AT24_MAGIC;
		uint8_t *ptr = (uint8_t *)&buf;
		while(toWrite)
		{
			written_now = write(fd, ptr + written, toWrite);
			if(written_now == -1)
			{
				fprintf(stderr, "[AT24 DRIVER] Error writing config: %s\n", strerror(errno));
				goto cleanup_save;
			}
			written += written_now;
			toWrite -= written_now;
		}
	}
	else
		lseek(fd, SEEK_SET, 8);

	if(at24_version != AT24_CUR_VER)
	{
		uint8_t v = AT24_CUR_VER;
		written_now = write(fd, &v, 1);
		if(written_now == -1)
		{
			fprintf(stderr, "[AT24 DRIVER] Error writing config: %s\n", strerror(errno));
			goto cleanup_save;
		}
		at24_version = AT24_CUR_VER;
	}
	else
		lseek(fd, SEEK_CUR, 1);

	uint8_t *ptr;
	for(int i = 0; i < 4; i++)
	{
		if(!at24_changed[i])
		{
			lseek(fd, SEEK_CUR, sizeof(CONFIG_MODE));
			continue;
		}

		toWrite = sizeof(CONFIG_MODE);
		written = 0;
		ptr = (uint8_t *)(at24_config + i);
		while(toWrite)
		{
			written_now = write(fd, ptr + written, toWrite);
			if(written_now == -1)
			{
				fprintf(stderr, "[AT24 DRIVER] Error writing config: %s\n", strerror(errno));
				goto cleanup_save;
			}
			written += written_now;
			toWrite -= written_now;
		}
		at24_changed[i] = false;
	}

	if(!at24_magic_correct)
	{
		toWrite = 8;
		written = 0;
		uint64_t buf = AT24_MAGIC;
		ptr = (uint8_t *)&buf;
		while(toWrite)
		{
			written_now = write(fd, ptr + written, toWrite);
			if(written_now == -1)
			{
				fprintf(stderr, "[AT24 DRIVER] Error writing config: %s\n", strerror(errno));
				goto cleanup_save;
			}
			written += written_now;
			toWrite -= written_now;
		}
		at24_magic_correct = true;
	}


	close(fd);
	return true;

cleanup_save:
	close(fd);
	return false;
}

void deinitConfig()
{
	saveConfig();
}

CONFIG_MODE configGetMode(unsigned int index)
{
	return at24_config[index];
}
