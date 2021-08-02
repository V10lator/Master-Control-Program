#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "mcp23017.h"

static int mcpFd;
static const uint8_t mcp23017_addy[1] = { 0x12 };
static uint16_t mcp_old_state = 0x0000;

bool initialize_mcp23017()
{
	mcpFd = open("/dev/i2c-1", O_RDWR);
	if(mcpFd < 0)
	{
		fprintf(stderr, "[MCP20317 DRIVER] Errorpening i2c!\n");
		return false;
	}

	if(ioctl(mcpFd, I2C_SLAVE, 0x20) < 0)
	{
		fprintf(stderr, "[MCP20317 DRIVER] Error opening MCP23017!\n");
		close_mcp23017();
		return false;
	}

	uint8_t buffer[] = { 0x15,
		0x00, 0xFF,	// 0x00 - Set ports as inputs (IODIR)
		0xFF, 0xFF,	// 0x02 - Set active-low (IOPOL)
		0x00, 0x00,	// 0x04 - Disable interrupts (INTEN)
		0x00, 0x00, 	// 0x06 - Def values (DEFVAL unused)
		0xFF, 0xFF,	// 0x08 - TODO: INTCON
		0x00, 0x00,	// 0x0A - TODO: IOCON
		0xFF, 0xFF,	// 0x0C - Enable pullups (GPPU)
		0x00, 0x00, 	// 0x0E - TODO: INTF
		0x00, 0x00,	// 0x10 - TODO: INTCAP
		0x00, 0x00,	// 0x12 - Clear GPIO values (GPIO)
		0x00, 0x00, 	// 0x14 - Clear latches (OLAT)
	};

	if(write(mcpFd, buffer, 2) != 2)
	{
		fprintf(stderr, "[MCP20317 DRIVER] Error settig 16 bit mode!\n");
		close_mcp23017();
		return false;
	}

	buffer[0] = 0x00;
	buffer[1] = 0xFF;
	if(write(mcpFd, buffer, 23) != 23)
	{
		fprintf(stderr, "[MCP20317 DRIVER] Error sending init sequence!\n");
		close_mcp23017();
		return false;
	}

	return true;
}

void close_mcp23017()
{
	close(mcpFd);
}

uint16_t read_mcp23017()
{
	if(write(mcpFd, mcp23017_addy, 1) != 1)
	{
		fprintf(stderr, "[MCP20317 DRIVER] Error reading GPIO pins (1)!\n");
		return mcp_old_state;
	}

	uint16_t ret;
	if(read(mcpFd, &ret, 2) != 2)
	{
		fprintf(stderr, "[MCP20317 DRIVER] Error reading GPIO pins (2)!\n");
		return mcp_old_state;
	}

	mcp_old_state = ret;
	return ret;
}
