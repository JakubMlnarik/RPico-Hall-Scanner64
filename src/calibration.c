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
void calibration_update_keys_limits(void) {
    static uint16_t buffer[READOUTS_PER_SAMPLE][HALL_SCANNER_TOTAL_CHANNELS] = {0};

    static int readout_counter = 0; // This holds readout number for average calculation
    
    // Read all sensors into buffer
    hall_scanner_read_all(buffer[readout_counter]);

    // Calculate average and set limits every N iteration
    if (readout_counter == (READOUTS_PER_SAMPLE - 1)) {
        for (int ch; ch<HALL_SCANNER_TOTAL_CHANNELS; ch++) {
            uint32_t sum = 0;
            for (int r; r<READOUTS_PER_SAMPLE; r++) {
                sum = sum + buffer[r][ch]; 
            }
            uint16_t res = (uint16_t)(sum / READOUTS_PER_SAMPLE);
            // Set limits
            if (keys_max_voltage[ch] < res) keys_max_voltage[ch] = res;
            if (keys_min_voltage[ch] > res) keys_min_voltage[ch] = res;

            //temprary oprint
            if (ch<4) {
                printf("Channel: %d, Max: %d, Min: %d\n", ch, keys_max_voltage[ch], keys_min_voltage[ch]);
            }
        }
    }

    readout_counter = (readout_counter + 1) % READOUTS_PER_SAMPLE;
}