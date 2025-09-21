#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "mcp3008_reader.h"
#include <stdio.h>

void adc_task() {
    while (true) {
        mcp3008_read_all(mcp3008_values);
        sleep_ms(10); // Sampling interval
    }
}

int main() {
    stdio_init_all();
    mcp3008_reader_init();
    multicore_launch_core1(adc_task);
    while (true) {
        sleep_ms(1000);
        critical_section_enter_blocking(&mcp3008_critical_section);
        printf("ADC values:\n");
        for (int i = 0; i < MCP3008_TOTAL_CHANNELS; ++i) {
            printf("%d: %u\n", i, mcp3008_values[i]);
        }
        critical_section_exit(&mcp3008_critical_section);
        printf("----\n");
    }
}
