#include "calibration.h"

// Key limits - this holds dynamically updated max and min voltage for a key
// Data collected during calibration session
// When a key is pressed the voltage is HIGHER than the key is released
uint16_t keys_max_voltage[HALL_SCANNER_TOTAL_CHANNELS];
uint16_t keys_min_voltage[HALL_SCANNER_TOTAL_CHANNELS];

// Global variable identifying state of the calibration process
typedef enum {
    Started, // Calibration just started
    InProgress, // Calibration is in progress
    Stopped, // Calibration is done - a dialog is finished
    Finished, // Calibration was used and effectively saved
} CalibrationProcessState;

// Default state is "Finished"
CalibrationProcessState state = Finished;

void calibration_start(void) {
    state = Started;
    // Max values are getting higher during calibration, so the init value is low
    for (int i=0; i<HALL_SCANNER_TOTAL_CHANNELS; i++) {
        keys_max_voltage[i] = MAX_INIT_VALUE;
        keys_min_voltage[i] = MIN_INIT_VALUE;
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
    if (actual_time_ms - last_event_time >= SAMPLING_INTERVAL_MS) {
        for (int ch=0; ch<HALL_SCANNER_TOTAL_CHANNELS; ch++) {
            uint16_t res = (uint16_t)(voltage_sum[ch] / readout_counter);
            // Clear voltage_sum buffer after use
            voltage_sum[ch] = 0;

            // Set limits
            if (keys_max_voltage[ch] < res) keys_max_voltage[ch] = res;
            if (keys_min_voltage[ch] > res) keys_min_voltage[ch] = res;
        }

        //TODO: remove prints
        printf("Counter: %d, Max: %d, Min: %d", readout_counter, keys_max_voltage[0], keys_min_voltage[0]);

        readout_counter = 0;
        last_event_time = actual_time_ms;
    }
}