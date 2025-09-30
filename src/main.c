#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "mcp3008_reader.h"
#include "settings.h"
#include "midi.h"
#include <stdio.h>

////////////////////////////
// global variables
////////////////////////////
SETTINGS main_settings;

// critical code protection from concurrency
critical_section_t cs_lock;

// shared output buffer for MIDI events
queue_t shared_midi_buff;

// Wrapper for midi_process to run on core1
void midi_process_core1_entry() {
    midi_process(&main_settings, &cs_lock, &shared_midi_buff);
}
int main() {
    // Initialize MIDI queue and critical section lock
    critical_section_init(&cs_lock);
    queue_init(&shared_midi_buff, sizeof(uint8_t), MIDI_BUFFER_SIZE);

    // Load settings from flash
    settings_load(&main_settings);

    // Print SETTINGS structure
    printf("SETTINGS:\n");
    printf("  magic_1: %u\n", main_settings.magic_1);
    printf("  magic_2: %u\n", main_settings.magic_2);
    printf("  magic_3: %u\n", main_settings.magic_3);
    printf("  magic_4: %u\n", main_settings.magic_4);
    printf("  fast_midi: %u\n", main_settings.fast_midi);
    printf("  m_ch: %u\n", main_settings.m_ch);
    printf("  m_base: %u\n", main_settings.m_base);
    printf("  sensitivity: %u\n", main_settings.sensitivity);
    printf("  threshold: %u\n", main_settings.threshold);
    printf("  coef_A: [");
    for (int i = 0; i < MIDI_NO_TONES; ++i) {
        printf("%u%s", main_settings.coef_A[i], (i < MIDI_NO_TONES-1) ? "," : "]\n");
    }
    printf("  coef_B: [");
    for (int i = 0; i < MIDI_NO_TONES; ++i) {
        printf("%u%s", main_settings.coef_B[i], (i < MIDI_NO_TONES-1) ? "," : "]\n");
    }

    mcp3008_reader_init();

    // Launch midi_process on core1
    multicore_launch_core1(midi_process_core1_entry);

    // Main core: read and print MIDI buffer
    while (true) {
        // Lock critical section before accessing the queue
        critical_section_enter_blocking(&cs_lock);
        while (!queue_is_empty(&shared_midi_buff)) {
            uint8_t val;
            if (queue_try_remove(&shared_midi_buff, &val)) {
                printf("MIDI: %u\n", val);
            }
        }
        critical_section_exit(&cs_lock);
        sleep_ms(1000);
    }

    return 0;
}
