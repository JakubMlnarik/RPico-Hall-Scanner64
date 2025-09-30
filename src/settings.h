
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

    // Coefficient to calculate position from voltage
    uint16_t coef_A[MIDI_NO_TONES];
    uint16_t coef_B[MIDI_NO_TONES];

    // Sensitivity factor - to calculate velocity from voltage difference (0-100%)
    uint8_t sensitivity;

    // Threshold for note on detection (0-100%): 0 = always on, 100 = never on
    uint8_t threshold;
} SETTINGS;

// default values
#define SETTINGS_MAGIC_1 1
#define SETTINGS_MAGIC_2 2
#define SETTINGS_MAGIC_3 3
#define SETTINGS_MAGIC_4 4
#define SETTINGS_FAST_MIDI_DEF 0
#define SETTINGS_M_CH_DEF 0
#define SETTINGS_M_BASE_DEF 36
#define SETTINGS_SENSITIVITY_DEF 50
#define SETTINGS_THRESHOLD_DEF 30
#define SETTINGS_COEF_A_DEF 500
#define SETTINGS_COEF_B_DEF 2000


extern void settings_load(SETTINGS *set);
extern void settings_save(SETTINGS *set);