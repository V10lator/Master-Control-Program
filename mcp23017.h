#pragma once

#include <stdbool.h>
#include <stdint.h>

bool initialize_mcp23017();
void close_mcp23017();
uint16_t read_mcp23017();
