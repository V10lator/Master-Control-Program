#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>

#include "config.h"

#define AT24_LENGTH 4096
#define AT24_MAGIC 0x1337BABE // TODO
#define AT24_CUR_VER 0

typedef struct // TODO: Make this __attribute__((__packed__) ?
{
	uint64_t magic;
	uint8_t version;
	CONFIG_MODE mode[4];
} AT24_CONFIG;

static volatile AT24_CONFIG *at24_config = MAP_FAILED;

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
	if(sizeof(AT24_CONFIG) > AT24_LENGTH)
	{
		fprintf(stderr, "[AT24 DRIVER] Sanity check: %lu > %i\n", sizeof(AT24_CONFIG), AT24_LENGTH);
		return false;
	}

	int fd = open("/sys/class/i2c-dev/i2c-1/device/1-0057/eeprom", O_RDWR | O_SYNC) ;
	if(fd < 0)
	{
		fprintf(stderr, "[AT24 DRIVER] Error opening sys file!\n");
		return false;
	}

	at24_config = (AT24_CONFIG *) mmap(0, AT24_LENGTH,
		PROT_READ | PROT_WRITE,
		MAP_SHARED | MAP_LOCKED,
		fd, 0
	);

	close(fd);

 	if(at24_config == MAP_FAILED)
	{
		fprintf(stderr,	"[AT24 DRIVER] Bad, mmap failed\n");
		return false;
	}

	if(at24_config->magic != AT24_MAGIC)
	{
		at24_config->magic = AT24_MAGIC;
		at24_config->version = AT24_CUR_VER;
		for(int i = 0; i < 4; i++)
			at24_config->mode[i] = defConf[i];
	}

	return true;
}

void deinitConfig()
{
	if(at24_config != MAP_FAILED)
	{
		munmap((void *)at24_config, AT24_LENGTH);
		at24_config = MAP_FAILED;
	}
}

CONFIG_MODE configGetMode(unsigned int index)
{
	return at24_config->mode[index];
}
