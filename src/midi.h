#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "pico/util/queue.h"
#include "pico/critical_section.h"
#include "hall_scanner.h"
#include "settings.h"
#include "midi_defs.h"

// filtering of analog values using moving average
#define MIDI_MA_COUNT 2  // Moving average window size

// Buffer for velocity calculation
#define MIDI_VELOCITY_BUFFER_SIZE 10

// NOTE ON / NOTE OFF hysteresis (in percentage of the total span of analog values)
#define MIDI_ON_OFF_HYSTERESIS_PERCENTAGE 10

// MIDI API
bool midi_send_msg(uint8_t *data, int no_bytes, critical_section_t *cs, queue_t *buff);
bool midi_send_note_on(uint8_t channel, uint8_t midi_base, int input, uint8_t velocity, critical_section_t *cs, queue_t *buff);
bool midi_send_note_off(uint8_t channel, uint8_t midi_base, int input, critical_section_t *cs, queue_t *buff);

// Process MIDI messages based on sensor inputs
void midi_process(SETTINGS *set, critical_section_t *cs, queue_t *buff);
