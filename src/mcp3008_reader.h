
#pragma once
#include <stdint.h>
#include "pico/critical_section.h"

#define MCP3008_NUM_CHIPS 8
#define MCP3008_CHANNELS_PER_CHIP 8
#define MCP3008_TOTAL_CHANNELS (MCP3008_NUM_CHIPS * MCP3008_CHANNELS_PER_CHIP)

extern volatile uint16_t mcp3008_values[MCP3008_TOTAL_CHANNELS];
extern critical_section_t mcp3008_critical_section;

#define MCP3008_NUM_CHIPS 8
#define MCP3008_CHANNELS_PER_CHIP 8
#define MCP3008_TOTAL_CHANNELS (MCP3008_NUM_CHIPS * MCP3008_CHANNELS_PER_CHIP)

// SPI0, chip selects: GP2, GP3, ... GP9
#define MCP3008_SPI_PORT spi0
#define MCP3008_CS_PINS {2, 3, 4, 5, 6, 7, 8, 9}

void mcp3008_reader_init(void);
void mcp3008_read_all(uint16_t values[MCP3008_TOTAL_CHANNELS]);
