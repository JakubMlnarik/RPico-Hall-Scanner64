#include "settings.h"

uint8_t *flash_target_contents = (uint8_t *) (XIP_BASE + SETTINGS_FLASH_TARGET_OFFSET);
uint8_t flash_buff[FLASH_PAGE_SIZE];

void settings_save(SETTINGS *set) {
    uint32_t ints = save_and_disable_interrupts();
    // Erase the last sector of the flash
    flash_range_erase(SETTINGS_FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);

    // write new settings
    memcpy(flash_buff, set, sizeof(SETTINGS));
    flash_range_program(SETTINGS_FLASH_TARGET_OFFSET, (uint8_t *)flash_buff, FLASH_PAGE_SIZE);
    restore_interrupts (ints);
}

void settings_load(SETTINGS *set) {
    memcpy(set, flash_target_contents, sizeof(SETTINGS));

    // validation of magic numbers
    if ( (set->magic_1 != SETTINGS_MAGIC_1)
        || (set->magic_2 != SETTINGS_MAGIC_2)
        || (set->magic_3 != SETTINGS_MAGIC_3)
        || (set->magic_4 != SETTINGS_MAGIC_4) ) {
            set->magic_1 = SETTINGS_MAGIC_1;
            set->magic_2 = SETTINGS_MAGIC_2;
            set->magic_3 = SETTINGS_MAGIC_3;
            set->magic_4 = SETTINGS_MAGIC_4;
            set->fast_midi = SETTINGS_FAST_MIDI_DEF;
            set->m_ch = SETTINGS_M_CH_DEF;
            set->m_base = SETTINGS_M_BASE_DEF;
            set->sensitivity = SETTINGS_SENSITIVITY_DEF;
            set->threshold = SETTINGS_THRESHOLD_DEF;
            for (int i = 0; i < MIDI_NO_TONES; ++i) {
                set->released_keys_voltage[i] = SETTINGS_RELEASED_KEYS_VOLTAGE_DEF;
                set->pressed_keys_voltage[i] = SETTINGS_PRESSED_KEYS_VOLTAGE_DEF;
            }
            settings_save(set);
    }
}