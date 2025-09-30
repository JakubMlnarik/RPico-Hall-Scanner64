#pragma once
#include <stdint.h>
#include "pico/critical_section.h"

// TODO: TEMPORARY - Changed from MCP3008 to MCP3208 for testing
#define MCP3008_NUM_CHIPS 8  // TODO: was MCP3008_NUM_CHIPS
#define MCP3008_CHANNELS_PER_CHIP 8  // TODO: was MCP3008_CHANNELS_PER_CHIP
#define MCP3008_TOTAL_CHANNELS (MCP3008_NUM_CHIPS * MCP3008_CHANNELS_PER_CHIP)  // TODO: was MCP3008_TOTAL_CHANNELS

// SPI0, chip selects: GP2, GP3, ... GP9
// TODO: TEMPORARY - Using MCP3208 (12-bit) instead of MCP3008 (10-bit)
#define MCP3008_SPI_PORT spi0
#define MCP3008_CS_PINS {2, 3, 4, 5, 6, 7, 8, 9}

void mcp3008_reader_init(void);
void mcp3008_read_all(uint16_t *values);
