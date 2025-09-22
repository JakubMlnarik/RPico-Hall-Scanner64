#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "pico/util/queue.h"
#include "pico/critical_section.h"
#include "mcp3008_reader.h"
#include "settings.h"
#include "midi_defs.h"

// MIDI API
bool midi_send_msg(uint8_t *data, int no_bytes, critical_section_t *cs, queue_t *buff);
bool midi_send_note_on(uint8_t channel, uint8_t midi_base, int input, uint8_t velocity, critical_section_t *cs, queue_t *buff);
bool midi_send_note_off(uint8_t channel, uint8_t midi_base, int input, critical_section_t *cs, queue_t *buff);

// Process MIDI messages based on sensor inputs
void midi_process(SETTINGS *set, critical_section_t *cs, queue_t *buff);
