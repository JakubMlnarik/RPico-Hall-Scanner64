#pragma once
#include <stdint.h>
#include "pico/critical_section.h"

#define HALL_SCANNER_NUM_AD_CHIPS 8
#define HALL_SCANNER_CHANNELS_PER_AD_CHIP 8

// SPI0, chip selects: GP2, GP3, ... GP9
// Using MCP3008 (10-bit)
#define HALL_SCANNER_SPI_PORT spi0
#define HALL_SCANNER_CS_PINS {2, 3, 4, 5, 6, 7, 8, 9}

void hall_scanner_init(void);
void hall_scanner_read_all(uint16_t *values, uint8_t count);
