#include "mcp3008_reader.h"
#include "pico/critical_section.h"
volatile uint16_t mcp3008_values[MCP3008_TOTAL_CHANNELS] = {0};
critical_section_t mcp3008_critical_section;
#include "hardware/spi.h"
#include "pico/stdlib.h"

static const uint8_t cs_pins[8] = MCP3008_CS_PINS;

void mcp3008_reader_init(void) {
    critical_section_init(&mcp3008_critical_section);
    spi_init(MCP3008_SPI_PORT, 1000 * 1000); // 1 MHz
    gpio_set_function(16, GPIO_FUNC_SPI); // MISO
    gpio_set_function(18, GPIO_FUNC_SPI); // SCK
    gpio_set_function(19, GPIO_FUNC_SPI); // MOSI
    for (int i = 0; i < 8; ++i) {
        gpio_init(cs_pins[i]);
        gpio_set_dir(cs_pins[i], GPIO_OUT);
        gpio_put(cs_pins[i], 1);
    }
}

static uint16_t mcp3008_read_channel(uint8_t chip, uint8_t channel) {
    uint8_t cs = cs_pins[chip / (MCP3008_NUM_CHIPS / 3)];
    gpio_put(cs, 0);
    uint8_t tx[] = {
        0x06 | ((channel & 0x04) >> 2),
        (channel & 0x03) << 6,
        0x00
    };
    uint8_t rx[3];
    spi_write_read_blocking(MCP3008_SPI_PORT, tx, rx, 3);
    gpio_put(cs, 1);
    uint16_t result = ((rx[1] & 0x0F) << 8) | rx[2];
    return result;
}

void mcp3008_read_all(uint16_t values[MCP3008_TOTAL_CHANNELS]) {
    critical_section_enter_blocking(&mcp3008_critical_section);
    for (uint8_t chip = 0; chip < MCP3008_NUM_CHIPS; ++chip) {
        for (uint8_t ch = 0; ch < MCP3008_CHANNELS_PER_CHIP; ++ch) {
            uint16_t val = mcp3008_read_channel(chip, ch);
            values[chip * MCP3008_CHANNELS_PER_CHIP + ch] = val;
            mcp3008_values[chip * MCP3008_CHANNELS_PER_CHIP + ch] = val;
        }
    }
    critical_section_exit(&mcp3008_critical_section);
}
