#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "hall_scanner.h"
#include "settings.h"
#include "midi_defs.h"

// Max values are getting higher during calibration, so the init value is low
#define CALIBRATION_MAX_INIT_VALUE 0
#define CALIBRATION_MIN_INIT_VALUE 3500

// To calculate arithmetic average from unknown number of values in given period 
#define CALIBRATION_SAMPLING_INTERVAL_MS 500

// Minimal number of samples collected to calculate the average
#define CALIBRATION_MINIMAL_SAMPLES_COUNT 3

// TODO: adjust better
// Minimal valid delta between max and min measured value. It enables to recognize if a tone was pressed during calibration. 
#define CALIBRATION_MINIMAL_DELTA 200

// Start calibration process
void calibration_init(void);

// Updates max and min voltage per ecah key during the calibration session
void calibration_update_keys_limits(uint32_t actual_time_ms);

// Calculates and updates voltage thershold for each key
// It should be invoked when the calibration is used and saved
void calibration_calculate_and_save(SETTINGS *set);
