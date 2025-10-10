#pragma once
#include <stdint.h>
#include "pico/critical_section.h"

// Changed back from MCP3208 to MCP3008
#define HALL_SCANNER_NUM_AD_CHIPS 8
#define HALL_SCANNER_CHANNELS_PER_AD_CHIP 8
#define HALL_SCANNER_TOTAL_CHANNELS (HALL_SCANNER_NUM_AD_CHIPS * HALL_SCANNER_CHANNELS_PER_AD_CHIP)

// SPI0, chip selects: GP2, GP3, ... GP9
// Using MCP3008 (10-bit) instead of MCP3208 (12-bit)
#define HALL_SCANNER_SPI_PORT spi0
#define HALL_SCANNER_CS_PINS {2, 3, 4, 5, 6, 7, 8, 9}

void hall_scanner_init(void);
void hall_scanner_read_all(uint16_t *values);
