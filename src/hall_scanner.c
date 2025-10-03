#include "hall_scanner.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"

// TODO: TEMPORARY - Using MCP3208 instead of MCP3008
static const uint8_t cs_pins[8] = HALL_SCANNER_CS_PINS;

void hall_scanner_init(void) {
    // No shared state, no critical section needed
    spi_init(HALL_SCANNER_SPI_PORT, 1000 * 1000); // 1 MHz
    gpio_set_function(16, GPIO_FUNC_SPI); // MISO
    gpio_set_function(18, GPIO_FUNC_SPI); // SCK
    gpio_set_function(19, GPIO_FUNC_SPI); // MOSI
    for (int i = 0; i < 8; ++i) {
        gpio_init(cs_pins[i]);
        gpio_set_dir(cs_pins[i], GPIO_OUT);
        gpio_put(cs_pins[i], 1);
    }
}

// TODO: TEMPORARY - Modified for MCP3208 (12-bit instead of 10-bit)
static uint16_t hall_scanner_read_channel(uint8_t chip, uint8_t channel) {
    uint8_t cs = cs_pins[chip];  // Fixed: direct mapping instead of division
    
    gpio_put(cs, 0);
    sleep_us(1);  // Ensure tCSS timing (1µs >> 100ns required by MCP3208)
    
    // MCP3208 SPI command format (same as MCP3008 but expecting 12-bit result)
    uint8_t tx[] = {
        0x06 | ((channel & 0x04) >> 2),  // Start bit + SGL/DIFF + D2
        (channel & 0x03) << 6,          // D1 + D0 + don't care bits
        0x00                            // Don't care
    };
    uint8_t rx[3];
    spi_write_read_blocking(HALL_SCANNER_SPI_PORT, tx, rx, 3);
    
    sleep_us(1);  // Ensure tCSH timing (1µs >> 100ns required by MCP3208)
    gpio_put(cs, 1);
    // MCP3208 returns 12-bit result (instead of 10-bit for MCP3008)
    uint16_t result = ((rx[1] & 0x0F) << 8) | rx[2];  // Extract 12-bit value
    return result;
}

void hall_scanner_read_all(uint16_t *values) {
    for (uint8_t chip = 0; chip < HALL_SCANNER_NUM_AD_CHIPS; ++chip) {
        for (uint8_t ch = 0; ch < HALL_SCANNER_CHANNELS_PER_AD_CHIP; ++ch) {
            values[chip * HALL_SCANNER_CHANNELS_PER_AD_CHIP + ch] = hall_scanner_read_channel(chip, ch);
        }
    }
}
