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

static int mcpFd = NULL;
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

	// [ Bank | MIRROR | SEQOP | DISSLW | HAEN | ODR | INTPOL | NOT IMPL]
	// [ BIT7 |        |       |        |      |     |        |   BIT0  ]
	// [  0   |   1    |  0    |   0    |  0   |  1  |    0   |    0    ]
	uint8_t iocon = 1 << 2;
	iocon |= 1 << 6;

	uint8_t buffer[] = { 0x15,
		iocon, 0xFF,	// 0x00 - Set ports as inputs (IODIR)
		0xFF, 0xFF,	// 0x02 - Set active-low (IOPOL)
		0x00, 0x00,	// 0x04 - Disable interrupts (INTEN)
		0x00, 0x00, 	// 0x06 - Def values (DEFVAL unused)
		0x00, 0x00,	// 0x08 - Compare against last value or def? (INTCON unused)
		iocon, iocon,	// 0x0A - IOCON
		0xFF, 0xFF,	// 0x0C - Enable pullups (GPPU)
		0x00, 0x00, 	// 0x0E - INTF - READ ONLY!
		0x00, 0x00,	// 0x10 - INTCAP -READ ONLY!
		0x00, 0x00,	// 0x12 - Clear GPIO values (GPIO)
		0x00, 0x00, 	// 0x14 - Clear latches (OLAT)
	};

	// Set 16 bit mode from 8 bit
	if(write(mcpFd, buffer, 2) != 2)
	{
		fprintf(stderr, "[MCP20317 DRIVER] Error settig 16 bit mode (1)!\n");
		close_mcp23017();
		return false;
	}

	// Just to make sure: Set 16 bit mode from 16 bit
	buffer[0] = 0x0A;
	if(write(mcpFd, buffer, 2) != 2)
	{
		fprintf(stderr, "[MCP20317 DRIVER] Error settig 16 bit mode (2)!\n");
		close_mcp23017();
		return false;
	}

	// Now that we're in 16 bit mode: Overwrite ALL registers (full software reset)
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
	if(mcpFd != NULL)
	{
		close(mcpFd);
		mcpFd = NULL;
	}
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
