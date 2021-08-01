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
static const uint8_t mcp23017_addy = 0x12;
static uint16_t mcp_old_state = 0x0000;

bool initialize_mcp23017()
{
	printf("[MCP20317 DRIVER] Opening i2c... ");
	fflush(stdout);
	mcpFd = open("/dev/i2c-1", O_RDWR);
	if(mcpFd < 0)
	{
		printf("[ERROR]\n");
		return false;
	}

	printf("[OK]\n[MCP20317 DRIVER] Opening MCP23017... ");
	fflush(stdout);
	if(ioctl(mcpFd, I2C_SLAVE, 0x20) < 0)
	{
		printf("[ERROR\n");
		close_mcp23017();
		return false;
	}

	printf("[OK]\n[MCP20317 DRIVER] Setting 16 bit mode... ");
	fflush(stdout);
	uint8_t buffer[3];

	buffer[0] = 0x15;
	buffer[1] = 0x00;

	if(write(mcpFd, buffer, 2) != 2)
	{
		printf("[ERROR]\n");
		close_mcp23017();
		return false;
	}

	// Disable interrupts (INTEN)
	printf("[OK]\n[MCP20317 DRIVER] Disabling interrupts... ");
	fflush(stdout);
	buffer[0] = 0x04;
	buffer[2] = 0x00;
	if(write(mcpFd, buffer, 3) != 3)
        {
                printf("[ERROR]\n");
		close_mcp23017();
                return false;
        }

	// Set ports as inputs (IODIR)
	printf("[OK]\n[MCP20317 DRIVER] Setting GPIO pins as input pins... ");
	fflush(stdout);
	buffer[0] = 0x00;
	buffer[1] = buffer[2] = 0xFF;
	if(write(mcpFd, buffer, 3) != 3)
        {
                printf("[ERROR]\n");
		close_mcp23017();
                return false;
        }

	printf("[OK]\n[MCP20317 DRIVER] Enabling pullups... ");
	fflush(stdout);
	// Enable pullups (GPPU)
	buffer[0] = 0x0C;
	if(write(mcpFd, buffer, 3) != 3)
        {
                printf("[ERROR]\n");
		close_mcp23017();
                return false;
        }

	// Set active-low (IOPOL)
	printf("[OK]\n[MCP20317 DRIVER] Setting active-low... ");
	fflush(stdout);
	buffer[0] = 0x02;
	if(write(mcpFd, buffer, 3) != 3)
        {
                printf("[ERROR]\n");
		close_mcp23017();
                return false;
        }
        printf("[OK]\n");

	return true;
}

void close_mcp23017()
{
	close(mcpFd);
}

uint16_t read_mcp23017()
{
	if(write(mcpFd, &mcp23017_addy, 1) != 1)
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
