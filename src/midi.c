#include "midi.h"

//--- MIDI message sending functions ---
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

//--- Moving Average to filter analog values for multiple channels ---
typedef struct {
    uint16_t buffer[MIDI_MA_COUNT];
    int index;
    int count;
    uint32_t sum;
} MovingAverage;

// Array of moving averages for all channels
static MovingAverage channel_filters[MIDI_NO_TONES];

// Initialize all moving average structures
void init_all_moving_averages(void) {
    for (int ch = 0; ch < MIDI_NO_TONES; ch++) {
        MovingAverage *ma = &channel_filters[ch];
        for (int i = 0; i < MIDI_MA_COUNT; i++) {
            ma->buffer[i] = 0;
        }
        ma->index = 0;
        ma->count = 0;
        ma->sum = 0;
    }
}

uint16_t moving_average_add(MovingAverage *ma, uint16_t new_value) {
    // Input validation
    if (!ma) return 0;
    
    // Subtract the oldest value from sum
    ma->sum -= ma->buffer[ma->index];
    
    // Add the new value to the buffer and sum
    ma->buffer[ma->index] = new_value;
    ma->sum += new_value;
    
    // Update index (circular buffer)
    ma->index = (ma->index + 1) % MIDI_MA_COUNT;
    
    // Update count (saturate at buffer size)
    if (ma->count < MIDI_MA_COUNT) {
        ma->count++;
    }
    
    // Return the average (add 0.5 for rounding)
    return (uint16_t)((ma->sum + ma->count/2) / ma->count);
}

// Filter all channels at once
void filter_all_channels(uint16_t *raw_values, uint16_t *filtered_values) {
    for (int i = 0; i < MIDI_NO_TONES; i++) {
        filtered_values[i] = moving_average_add(&channel_filters[i], raw_values[i]);
    }
}

//--- State structures holding everything need for calculation NOTE ON/OFF and velocity ---
typedef enum {
    KEY_UNDEFINED,
    KEY_RELEASED,
    KEY_PRESSED
} KeyPosition;

typedef struct {
    uint16_t velocity_buffer[MIDI_VELOCITY_BUFFER_SIZE];
    int index;
    uint16_t on_threshold;
    uint16_t off_threshold;
    uint16_t released_voltage;
    KeyPosition position;
} KeyState;

// Array of key states for all channels
static KeyState key_states[MIDI_NO_TONES];

// Initialize all key states
void init_all_key_states(SETTINGS *set) {
    for (int ch = 0; ch < MIDI_NO_TONES; ch++) {
        KeyState *ks = &key_states[ch];
        
        // Initialize velocity buffer with released voltage instead of zeros
        for (int i = 0; i < MIDI_VELOCITY_BUFFER_SIZE; i++) {
            ks->velocity_buffer[i] = set->released_voltage[ch];
        }
        
        ks->index = 0;
        ks->position = KEY_RELEASED;
        ks->released_voltage = set->released_voltage[ch];
        
        // OFF threshold is in the middle between pressed and released voltage
        ks->off_threshold = (set->pressed_voltage[ch] + set->released_voltage[ch]) / 2;
        // ON threshold is OFF threshold plus hysteresis
        uint16_t delta = set->pressed_voltage[ch] - set->released_voltage[ch];
        ks->on_threshold = ks->off_threshold + (delta * MIDI_ON_OFF_HYSTERESIS_PERCENTAGE) / 100;
    }
}

// Update single key state - capture velocity data during key press motion
void update_key_state(int channel, uint16_t value) {
    KeyState *ks = &key_states[channel];
    
    KeyPosition old_position = ks->position;
    
    // Determine new position
    if (value > ks->off_threshold) {
        ks->position = KEY_RELEASED;
        // Reset velocity buffer when key is released
        if (old_position != KEY_RELEASED) {
            for (int i = 0; i < MIDI_VELOCITY_BUFFER_SIZE; i++) {
                ks->velocity_buffer[i] = ks->released_voltage;
            }
            ks->index = 0;
        }
    } else if (value < ks->on_threshold) {
        ks->position = KEY_PRESSED;
    } else {
        ks->position = KEY_UNDEFINED;
    }
    
    // Update velocity buffer during key press motion (when moving away from released state)
    // This captures the velocity data during the entire key press, not just when pressed
    if (value < ks->released_voltage) {  // Key is being pressed (voltage going down)
        ks->velocity_buffer[ks->index] = value;
        ks->index = (ks->index + 1) % MIDI_VELOCITY_BUFFER_SIZE;
    }
}

// Function updating key state using moving average filtered values
void update_all_key_states(void) {
    uint16_t raw[MIDI_NO_TONES] = {0};
    hall_scanner_read_all(raw, MIDI_NO_TONES);
    uint16_t filtered[MIDI_NO_TONES] = {0};
    filter_all_channels(raw, filtered);

    for (int ch = 0; ch < MIDI_NO_TONES; ch++) {
        KeyState *ks = &key_states[ch];
        // Update the key state based on the filtered value
        update_key_state(ch, filtered[ch]);
    }
}

// Calculate velocity based on integration of area below released voltage (inverted)
uint8_t calculate_velocity(int channel) {
    KeyState *ks = &key_states[channel];
    
    // Calculate the total area between released voltage and current readings
    // Since pressing the key decreases voltage, we measure area BELOW released voltage
    uint32_t total_area = 0;
    
    // Sum up all values below the released voltage in the buffer (inverted logic)
    for (int i = 0; i < MIDI_VELOCITY_BUFFER_SIZE; i++) {
        if (ks->velocity_buffer[i] < ks->released_voltage) {
            total_area += (ks->released_voltage - ks->velocity_buffer[i]);
        }
    }
    
    // Normalize by the actual voltage range for this specific key
    // This ensures consistent velocity response across all keys regardless of their individual voltage spans
    uint16_t voltage_range = ks->released_voltage - ks->off_threshold; // Inverted range
    if (voltage_range == 0) return 64; // Default velocity if no range
    
    // Your original normalization approach - much better!
    uint32_t velocity = total_area / voltage_range;
    
    // Ensure velocity is in valid MIDI range
    if (velocity > 127) velocity = 127;
    if (velocity < 1) velocity = 1;
    
    return (uint8_t)velocity;
}

//-- Process MIDI messages --
void midi_process(SETTINGS *set, critical_section_t *cs, queue_t *buff) {
    // Note ON/OFF state tracking
    static bool note_on_sent[MIDI_NO_TONES] = {false};

    init_all_moving_averages();
    init_all_key_states(set);

    while (true) {
        update_all_key_states();

        for (int i = 0; i < MIDI_NO_TONES; ++i) {
            if (key_states[i].position == KEY_PRESSED && note_on_sent[i] == false) {
                uint8_t velocity = calculate_velocity(i);
                printf("NOTE ON: %d, Velocity: %d\n", i, velocity);
                midi_send_note_on(set->m_ch, set->m_base, i, velocity, cs, buff);
                note_on_sent[i] = true;
            } else if (key_states[i].position == KEY_RELEASED && note_on_sent[i] == true) {
                printf("NOTE OFF: %d\n", i);
                midi_send_note_off(set->m_ch, set->m_base, i, cs, buff);
                note_on_sent[i] = false;
            }
        }
    }
}