#include "hall_scanner.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"
#include <stdio.h>

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

static uint16_t mcp3008_read_channel(int chip_index, int channel) {
    // MCP3008 SPI protocol:
    // Send: 1 byte start (0x01), 1 byte command, 1 byte dummy
    // Command byte: bit 7 = start bit, bit 6 = single/diff, bits 5-3 = channel, bits 2-0 = don't care
    
    uint8_t tx_buf[3];
    uint8_t rx_buf[3];
    
    tx_buf[0] = 0x01;  // Start bit
    tx_buf[1] = 0x80 | (channel << 4);  // Single-ended mode + channel select
    tx_buf[2] = 0x00;  // Dummy byte
    
    gpio_put(cs_pins[chip_index], 0);  // Select chip
    spi_write_read_blocking(HALL_SCANNER_SPI_PORT, tx_buf, rx_buf, 3);
    gpio_put(cs_pins[chip_index], 1);  // Deselect chip
    
    // Extract 10-bit result from rx_buf[1] and rx_buf[2]
    // MCP3008 returns: rx_buf[1] = X X X X X b9 b8 b7, rx_buf[2] = b6 b5 b4 b3 b2 b1 b0 X
    uint16_t result = ((rx_buf[1] & 0x03) << 8) | rx_buf[2];
    
    return result;  // 10-bit value (0-1023)
}

void hall_scanner_read_all(uint16_t *values, uint8_t count) {
    static uint32_t call_counter = 0;
    uint8_t read_count = 0;
    for (uint8_t chip = 0; chip < HALL_SCANNER_NUM_AD_CHIPS && read_count < count; ++chip) {
        for (uint8_t ch = 0; ch < HALL_SCANNER_CHANNELS_PER_AD_CHIP && read_count < count; ++ch) {
            values[read_count] = mcp3008_read_channel(chip, ch);
            read_count++;
        }
    }
    
    call_counter++;
    if (call_counter % 100 == 0) {
        printf("Values: ");
        for (uint8_t i = 0; i < read_count; ++i) {
            printf("[%u]=%u ", i, values[i]);
        }
        printf("\n");
    }
}
