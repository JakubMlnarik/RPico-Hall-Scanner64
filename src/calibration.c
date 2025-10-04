#include "calibration.h"

// Key limits - this holds dynamically updated max and min voltage for a key
// Data collected during calibration session
// When a key is pressed the voltage is HIGHER than the key is released
uint16_t keys_max_voltage[HALL_SCANNER_TOTAL_CHANNELS];
uint16_t keys_min_voltage[HALL_SCANNER_TOTAL_CHANNELS];

void calibration_init(void) {
    // Max values are getting higher during calibration, so the init value is low
    for (int i=0; i<HALL_SCANNER_TOTAL_CHANNELS; i++) {
        keys_max_voltage[i] = CALIBRATION_MAX_INIT_VALUE;
        keys_min_voltage[i] = CALIBRATION_MIN_INIT_VALUE;
    }
}

// This is invoked from an access_point module loop
// Every iteration takes cca 100 ms
// It creates an average from 5 previous values
void calibration_update_keys_limits(uint32_t actual_time_ms) {
    static uint32_t voltage_sum[HALL_SCANNER_TOTAL_CHANNELS] = {0};
    static uint32_t readout_counter = 0; // This holds readout number for average calculation
    static uint32_t last_event_time = 0;
    
    // Read all sensors into buffer
    uint16_t curr[HALL_SCANNER_TOTAL_CHANNELS] = {0};
    hall_scanner_read_all(curr);
    
    for (int ch=0; ch<HALL_SCANNER_TOTAL_CHANNELS; ch++) {
        voltage_sum[ch] = voltage_sum[ch] + curr[ch];
    }

    readout_counter++;

    // triggers every SAMPLING_INTERVAL_MS
    if ((actual_time_ms - last_event_time >= CALIBRATION_SAMPLING_INTERVAL_MS) &&
            (readout_counter > CALIBRATION_MINIMAL_SAMPLES_COUNT)) {
        for (int ch=0; ch<HALL_SCANNER_TOTAL_CHANNELS; ch++) {
            uint16_t res = (uint16_t)(voltage_sum[ch] / readout_counter);
            // Clear voltage_sum buffer after use
            voltage_sum[ch] = 0;

            // Set limits
            if (keys_max_voltage[ch] < res) keys_max_voltage[ch] = res;
            if (keys_min_voltage[ch] > res) keys_min_voltage[ch] = res;
        }

        //TODO: remove prints
        printf("Counter: %d, Max: %d, Min: %d\n", readout_counter, keys_max_voltage[0], keys_min_voltage[0]);

        readout_counter = 0;
        last_event_time = actual_time_ms;
    }
}

void calibration_calculate_and_save(SETTINGS *set) {
    for (int t=0; t<MIDI_NO_TONES; t++) {
        // calculate delta voltage
        uint16_t voltage_delta = 0;
        if (keys_max_voltage[t] > keys_min_voltage[t]) {
            voltage_delta = keys_max_voltage[t] - keys_min_voltage[t];
        }

        // Tone pressed detection
        if (voltage_delta > CALIBRATION_MINIMAL_DELTA) {
            // Valid calibration
            set->voltage_threshold[t] = (keys_max_voltage[t] + keys_min_voltage[t]) / 2; // In the middle between limits
            set->voltage_span[t] = voltage_delta;
        }
    }

    printf("  voltage_threshold: [");
    for (int i = 0; i < MIDI_NO_TONES; ++i) {
        printf("%u%s", set->voltage_threshold[i], (i < MIDI_NO_TONES-1) ? "," : "]\n");
    }
    printf("  voltage_span: [");
    for (int i = 0; i < MIDI_NO_TONES; ++i) {
        printf("%u%s", set->voltage_span[i], (i < MIDI_NO_TONES-1) ? "," : "]\n");
    }

    settings_save(set);
}