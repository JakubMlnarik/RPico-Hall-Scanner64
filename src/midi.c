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
    static uint16_t prev[HALL_SCANNER_TOTAL_CHANNELS] = {0};
    uint16_t curr[HALL_SCANNER_TOTAL_CHANNELS] = {0};
    static bool on_state[HALL_SCANNER_TOTAL_CHANNELS] = {false};

    // FIFO buffer for last 15 values of channel 0
    #define FIFO_SIZE 15
    static uint16_t fifo[FIFO_SIZE] = {0};
    static int fifo_index = 0;
    static int fifo_count = 0;

    while (true) {
        hall_scanner_read_all(curr);

        // Update FIFO for channel 0
        fifo[fifo_index] = curr[0];
        fifo_index = (fifo_index + 1) % FIFO_SIZE;
        if (fifo_count < FIFO_SIZE) fifo_count++;

        for (int i = 0; i < HALL_SCANNER_TOTAL_CHANNELS; ++i) {
            if (i < MIDI_NO_TONES) { // I am interested only about the real physical sensors
                if (curr[i] > set->on_voltage_threshold[i] && prev[i] <= set->on_voltage_threshold[i] && on_state[i] == false) {
                    printf("NOTE ON: %d\n", i);
                    midi_send_note_on(set->m_ch, set->m_base, i, 127, cs, buff);
                    on_state[i] = true;

                    // Print FIFO buffer for channel 0 only when note on is detected
                    printf("Last %d voltages for channel 0: ", fifo_count);
                    for (int j = 0; j < fifo_count; ++j) {
                        int idx = (fifo_index + j) % FIFO_SIZE;
                        printf("%u ", fifo[idx]);
                    }
                    printf("\n");
                } else if (curr[i] <= set->off_voltage_threshold[i] && prev[i] > set->off_voltage_threshold[i] && on_state[i] == true) {
                    printf("NOTE OFF: %d\n", i);
                    midi_send_note_off(set->m_ch, set->m_base, i, cs, buff);
                    on_state[i] = false;
                }
            }
            prev[i] = curr[i];
        }
    }
}
