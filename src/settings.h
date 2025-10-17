
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

    // Voltage of the pressed key
    uint16_t pressed_voltage[MIDI_NO_TONES];

    // Voltage of the released key
    uint16_t released_voltage[MIDI_NO_TONES];

} SETTINGS;

// default values
#define SETTINGS_MAGIC_1 1
#define SETTINGS_MAGIC_2 2
#define SETTINGS_MAGIC_3 3
#define SETTINGS_MAGIC_4 4
#define SETTINGS_FAST_MIDI_DEF 0
#define SETTINGS_M_CH_DEF 0
#define SETTINGS_M_BASE_DEF 36
#define SETTINGS_RELEASED_VOLTAGE_DEF 500
#define SETTINGS_PRESSED_VOLTAGE_DEF 700

extern void settings_load(SETTINGS *set);
extern void settings_save(SETTINGS *set);