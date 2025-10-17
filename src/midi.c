#include "midi.h"

bool midi_send_msg(uint8_t *data, int no_bytes, critical_section_t *cs, queue_t *buff) {
    if (no_bytes <= 0 || no_bytes > MIDI_BUFFER_SIZE) return false;
    bool success = true;
    critical_section_enter_blocking(cs);
    for (int i = 0; i < no_bytes; ++i) {
        if (!queue_try_add(buff, &data[i])) {
            success = false;
            break;
        }
    }
    critical_section_exit(cs);
    return success;
}

bool midi_send_note_on(uint8_t channel, uint8_t midi_base, int input, uint8_t velocity, critical_section_t *cs, queue_t *buff) {
    if (velocity > 127) velocity = 127;
    if ((int)velocity < 0) velocity = 0;
    uint8_t msg[3];
    msg[0] = 0x90 | (channel & 0x0F); // Note On
    msg[1] = midi_base + input;
    msg[2] = velocity;
    return midi_send_msg(msg, 3, cs, buff);
}

bool midi_send_note_off(uint8_t channel, uint8_t midi_base, int input, critical_section_t *cs, queue_t *buff) {
    uint8_t msg[3];
    msg[0] = 0x80 | (channel & 0x0F); // Note Off
    msg[1] = midi_base + input;
    msg[2] = 0x00; // Velocity
    return midi_send_msg(msg, 3, cs, buff);
}

// Process MIDI messages based on sensor inputs
void midi_process(SETTINGS *set, critical_section_t *cs, queue_t *buff) {
    // Analog readouts
    static uint16_t prev[MIDI_NO_TONES] = {0};
    uint16_t curr[MIDI_NO_TONES] = {0};

    // Note ON/OFF thresholds
    uint16_t on_threshold[MIDI_NO_TONES];
    uint16_t off_threshold[MIDI_NO_TONES];
    for (int i = 0; i < MIDI_NO_TONES; ++i) {
        // OFF threshold is in the middle between pressed and released voltage
        off_threshold[i] = (set->pressed_voltage[i] + set->released_voltage[i]) / 2;
        // ON threshold is OFF threshold plus hysteresis
        uint16_t delta = set->pressed_voltage[i] - set->released_voltage[i];
        on_threshold[i] = off_threshold[i] + (delta * MIDI_ON_OFF_HYSTERESIS_PERCENTAGE) / 100;
    }

    // Note ON/OFF state tracking
    static bool on_state[MIDI_NO_TONES] = {false};

    // FIFO buffer for last 10 values of channel 0
    #define FIFO_SIZE 20
    static uint16_t fifo[FIFO_SIZE] = {0};
    static int fifo_index = 0;
    static int fifo_count = 0;

    while (true) {
        hall_scanner_read_all(curr);

        // Update FIFO for channel 0
        fifo[fifo_index] = curr[0];
        fifo_index = (fifo_index + 1) % FIFO_SIZE;
        if (fifo_count < FIFO_SIZE) fifo_count++;

        for (int i = 0; i < MIDI_NO_TONES; ++i) {
            if (curr[i] > on_threshold[i] && prev[i] <= on_threshold[i] && on_state[i] == false) {
                printf("NOTE ON: %d\n", i);
                midi_send_note_on(set->m_ch, set->m_base, i, 127, cs, buff);
                on_state[i] = true;

                uint32_t vel = 0;
                // Print velocity
                for (int j = 0; j < fifo_count; ++j) {
                    int idx = (fifo_index + j) % FIFO_SIZE;
                    vel = vel + fifo[idx];
                }
                printf("Velocity integral: %d\n", vel);
            } else if (curr[i] <= off_threshold[i] && prev[i] > off_threshold[i] && on_state[i] == true) {
                printf("NOTE OFF: %d\n", i);
                midi_send_note_off(set->m_ch, set->m_base, i, cs, buff);
                on_state[i] = false;
            }
            prev[i] = curr[i];
        }
    }
}
