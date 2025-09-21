#include "pico/stdlib.h"
#include "hardware/spi.h"
#include <stdio.h>

#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19

uint16_t read_adc(uint8_t channel) {
    uint8_t tx[] = {
        0x06 | ((channel & 0x04) >> 2),  // Start bit + single-ended + MSB of channel
        (channel & 0x03) << 6,           // Remaining channel bits
        0x00                             // Dummy byte
    };
    uint8_t rx[3];

    gpio_put(PIN_CS, 0);
    spi_write_read_blocking(SPI_PORT, tx, rx, 3);
    gpio_put(PIN_CS, 1);

    uint16_t result = ((rx[1] & 0x0F) << 8) | rx[2];
    return result;
}

int main() {
    stdio_init_all();

    spi_init(SPI_PORT, 1000 * 1000); // 1 MHz
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);

    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);

    while (true) {
        uint32_t start = time_us_32();
        for (uint8_t ch = 0; ch < 8; ch++) {
            uint16_t value = read_adc(ch);
            float voltage = value * 3.3f / 4095.0f;
            printf("CH%u: %u (%.2f V)\n", ch, value, voltage);
        }
        uint32_t end = time_us_32();
        uint32_t elapsed_us = end - start;
        printf("Block time: %u us\n", elapsed_us);
        printf("----\n");
        sleep_ms(1000);
    }
}
