#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hall_scanner.h"
#include "settings.h"
#include "midi.h"
#include "access_point.h"
#include <stdio.h>

////////////////////////////
// global variables
////////////////////////////
SETTINGS main_settings;

// critical code protection from concurrency
critical_section_t cs_lock;

// shared output buffer for MIDI events
queue_t shared_midi_buff;

// WiFi button configuration
#define WIFI_BUTTON_GPIO 22

// Wrapper for midi_process to run on core1
void midi_process_core1_entry() {
    midi_process(&main_settings, &cs_lock, &shared_midi_buff);
}

// Initialize and check WiFi button
bool init_wifi_button() {
    // Initialize GPIO 22 as input with pull-up
    gpio_init(WIFI_BUTTON_GPIO);
    gpio_set_dir(WIFI_BUTTON_GPIO, GPIO_IN);
    gpio_pull_up(WIFI_BUTTON_GPIO);
    
    // Small delay to let the GPIO settle
    sleep_ms(50);
    
    // Return true if button is pressed (GPIO is LOW when button pressed due to pull-up)
    return !gpio_get(WIFI_BUTTON_GPIO);
}
int main() {
    // Initialize stdio for USB serial communication
    stdio_init_all();
    
    // Wait a bit for USB to be ready
    sleep_ms(2000);
    
    printf("Starting RPico Hall Scanner...\n");
    
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
    printf("  voltage_threshold: [");
    for (int i = 0; i < MIDI_NO_TONES; ++i) {
        printf("%u%s", main_settings.voltage_threshold[i], (i < MIDI_NO_TONES-1) ? "," : "]\n");
    }
    printf("  voltage_span: [");
    for (int i = 0; i < MIDI_NO_TONES; ++i) {
        printf("%u%s", main_settings.voltage_span[i], (i < MIDI_NO_TONES-1) ? "," : "]\n");
    }

    hall_scanner_init();

    // Initialize and check WiFi button  
    // TODO  
    //if (init_wifi_button()) {
    if (true) {
        printf("WiFi button pressed - starting Access Point mode\n");
        
        // Load settings from flash (needed for access point)
        settings_load(&main_settings);
        
        // Start access point mode
        wifi_ap_proc(&main_settings);
        
        // Access point function runs indefinitely, so we won't reach here
        return 0;
    }

    // Launch midi_process on core1
    multicore_launch_core1(midi_process_core1_entry);

    // Main core: read and print MIDI buffer
    // while (true) {
    //     // Lock critical section before accessing the queue
    //     critical_section_enter_blocking(&cs_lock);
    //     while (!queue_is_empty(&shared_midi_buff)) {
    //         uint8_t val;
    //         if (queue_try_remove(&shared_midi_buff, &val)) {
    //             printf("MIDI: %u\n", val);
    //         }
    //     }
    //     critical_section_exit(&cs_lock);
    //     sleep_ms(1000);
    // }

    return 0;
}
