#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "hall_scanner.h"

// Max values are getting higher during calibration, so the init value is low
#define MAX_INIT_VALUE 0
#define MIN_INIT_VALUE 3500

// To calculate arithmetic average from unknown number of values in given period 
#define SAMPLING_INTERVAL_MS 500

// Start calibration process
void calibration_start(void);

// Updates max and min voltage per ecah key during the calibration session
void calibration_update_keys_limits(uint32_t actual_time_ms);

// Calibration process stopped
void calibration_stop(void);

// Calculates and updates voltage thershold for each key
// It should be invoked when the calibration is used and saved
void calibration_calculate_and_finish(uint16_t *values);
