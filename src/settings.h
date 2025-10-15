
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hardware/sync.h>

#include "pico/stdlib.h"
#include "hardware/flash.h"

#include "midi_defs.h"


// Last sector of Flash
#define SETTINGS_FLASH_TARGET_OFFSET (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)

typedef struct SETTINGS_ {
    // Magic numbers to verify valid settings in flash (first boot)
    uint8_t magic_1;
    uint8_t magic_2;
    uint8_t magic_3;
    uint8_t magic_4;

    // MIDI settings
    uint8_t fast_midi;  // Enable fast MIDI mode
    uint8_t m_ch;       // MIDI channel
    uint8_t m_base;     // Base MIDI note

    // Voltage threshold for note on detection
    uint16_t on_voltage_threshold[MIDI_NO_TONES];

    // Voltage threshold for note off detection
    uint16_t off_voltage_threshold[MIDI_NO_TONES];
} SETTINGS;

// default values
#define SETTINGS_MAGIC_1 1
#define SETTINGS_MAGIC_2 2
#define SETTINGS_MAGIC_3 3
#define SETTINGS_MAGIC_4 4
#define SETTINGS_FAST_MIDI_DEF 0
#define SETTINGS_M_CH_DEF 0
#define SETTINGS_M_BASE_DEF 36
// NOTE ON / NOTE OFF hysteresis (in percentage of the total span of analog values)
#define SETTINGS_ON_OFF_HYSTERESIS_PERCENTAGE 10
#define SETTINGS_ON_VOLTAGE_THRESHOLD_DEF 600
#define SETTINGS_OFF_VOLTAGE_THRESHOLD_DEF 590


extern void settings_load(SETTINGS *set);
extern void settings_save(SETTINGS *set);