/**
 * @file status_dispatcher.c
 * @brief MIDI Status Byte Dispatcher Module
 * 
 * This module handles incoming MIDI bytes and dispatches them based on their status.
 * It maintains state to properly handle different MIDI message types including:
 * - System Exclusive messages (variable length)
 * - Channel messages (1-3 bytes)
 * - System Real-time messages (single byte, highest priority)
 * 
 * The dispatcher ensures complete MIDI messages are queued together while
 * allowing real-time messages to interrupt and be sent immediately.
 * 
 * @author Jakub Mlnarik
 * @date 2025
 */

#include "status_dispatcher.h"

/**
 * @brief Status Dispatcher State Machine
 * 
 * Controls the parsing workflow and determines how to process incoming MIDI bytes.
 * The state machine ensures proper handling of different MIDI message formats:
 * 
 * State Transitions:
 * - Any status byte can change the state
 * - Data bytes maintain current state
 * - Real-time messages don't change state (processed immediately)
 */
typedef enum {
    Undefined,          ///< Initial/error state - no valid status received
    SysEx,             ///< System Exclusive mode - collecting variable-length data
    SysExEnd,          ///< SysEx termination state - last byte of SysEx message
    TwoDataBytes,      ///< Expecting 2 data bytes (e.g., Note On/Off, CC, Pitch Bend)
    OneDataByte,       ///< Expecting 1 data byte (e.g., Program Change, Song Select)
} StatusDispatcherState;

// ============================================================================
// Message Type Lookup Table
// ============================================================================

/**
 * @brief MIDI Message Information Structure
 * 
 * Defines the properties of each MIDI message type for efficient lookup.
 * This replaces long if-else chains with a table-driven approach.
 */
typedef struct {
    unsigned char status_mask;    ///< Mask to apply to status byte for comparison
    unsigned char status_value;   ///< Expected value after masking
    int data_bytes;              ///< Number of expected data bytes (-1 = unknown, -2 = variable)
    bool is_realtime;            ///< True if this is a real-time message
    const char* name;            ///< Human-readable name for debugging
} MidiMessageInfo;

/**
 * @brief MIDI Message Lookup Table
 * 
 * Ordered by priority and frequency of occurrence for optimal performance.
 * Real-time messages are checked first, then channel messages, then system messages.
 */
static const MidiMessageInfo midi_message_table[] = {
    // Real-time messages (highest priority) - exact match required
    {0xFF, MIDI_CLOCK,              0, true,  "Clock"},
    {0xFF, MIDI_START,              0, true,  "Start"},
    {0xFF, MIDI_CONTINUE,           0, true,  "Continue"},
    {0xFF, MIDI_STOP,               0, true,  "Stop"},
    {0xFF, MIDI_ACTIVE_SENSING,     0, true,  "Active Sensing"},
    {0xFF, MIDI_SYSTEM_RESET,       0, true,  "System Reset"},
    {0xFF, MIDI_TICK,               0, true,  "Tick"},
    {0xFF, MIDI_UNDEFINED_F9,       0, true,  "Undefined F9"},
    {0xFF, MIDI_UNDEFINED_FD,       0, true,  "Undefined FD"},
    
    // Channel messages (mask out channel number) - most common
    {0xF0, MIDI_NOTE_ON,            2, false, "Note On"},
    {0xF0, MIDI_NOTE_OFF,           2, false, "Note Off"},
    {0xF0, MIDI_CONTROL_CHANGE,     2, false, "Control Change"},
    {0xF0, MIDI_PITCH_BEND,         2, false, "Pitch Bend"},
    {0xF0, MIDI_AFTERTOUCH_POLY,    2, false, "Poly Aftertouch"},
    {0xF0, MIDI_PROGRAM_CHANGE,     1, false, "Program Change"},
    {0xF0, MIDI_AFTERTOUCH_CHANNEL, 1, false, "Channel Aftertouch"},
    
    // System Common messages - exact match required
    {0xFF, MIDI_SYSTEM_EXCLUSIVE,   -2, false, "System Exclusive Start"},
    {0xFF, MIDI_SYSTEM_EXCLUSIVE_END, 0, false, "System Exclusive End"},
    {0xFF, MIDI_SONG_POSITION,      2, false, "Song Position"},
    {0xFF, MIDI_TIME_CODE_QUARTER,  1, false, "Time Code Quarter Frame"},
    {0xFF, MIDI_SONG_SELECT,        1, false, "Song Select"},
    {0xFF, MIDI_TUNE_REQUEST,       0, true,  "Tune Request"},
    {0xFF, MIDI_UNDEFINED_F4,       0, true,  "Undefined F4"},
    {0xFF, MIDI_UNDEFINED_F5,       0, true,  "Undefined F5"},
};

#define MIDI_MESSAGE_TABLE_SIZE (sizeof(midi_message_table) / sizeof(midi_message_table[0]))

/**
 * @brief Look up MIDI message information from the lookup table
 * @param status_byte The MIDI status byte to look up
 * @return Pointer to MidiMessageInfo structure, or NULL if not found
 */
static const MidiMessageInfo* lookup_midi_message(unsigned char status_byte) {
    for (size_t i = 0; i < MIDI_MESSAGE_TABLE_SIZE; i++) {
        if ((status_byte & midi_message_table[i].status_mask) == midi_message_table[i].status_value) {
            return &midi_message_table[i];
        }
    }
    return NULL;  // Unknown message type
}

/**
 * @brief Determines the new dispatcher state based on incoming MIDI byte
 * 
 * This function analyzes the incoming byte using a lookup table approach
 * for better performance and maintainability. It handles:
 * - Status byte detection (MSB = 1)
 * - System Exclusive message boundaries
 * - Message type classification using lookup table
 * 
 * @param byte Pointer to the incoming MIDI byte
 * @return StatusDispatcherState The new state for the dispatcher
 * 
 * @note Uses static variable to maintain state between calls
 * @note Real-time messages (0xF8-0xFF) don't change parsing state
 */
StatusDispatcherState setState(unsigned char *byte) {
    static StatusDispatcherState state;  // Persistent state between function calls

    // Check if this is a status byte (MSB = 1, indicates command/status)
    if (is_status_byte(*byte)) {

        // Handle System Exclusive start - enter SysEx collection mode
        if ((*byte == MIDI_SYSTEM_EXCLUSIVE) && (state != SysEx)) {
            state = SysEx;
            return state;
        }

        // During SysEx mode, only process the termination byte (0xF7)
        // All other bytes (including status bytes) are treated as SysEx data
        if (state == SysEx) {
            if (*byte == MIDI_SYSTEM_EXCLUSIVE_END) {
                state = SysExEnd;  // Mark end of SysEx message
            }
            return state;  // Stay in SysEx mode until F7 received
        }

        // SysExEnd is a single-cycle state - reset to undefined after processing
        // This should not occur in normal operation
        if (state == SysExEnd) {
            state = Undefined;
            return state;
        }

        // Use lookup table to determine message type and expected data bytes
        const MidiMessageInfo* msg_info = lookup_midi_message(*byte);
        if (msg_info != NULL) {
            // Determine state based on expected data byte count
            switch (msg_info->data_bytes) {
                case 0:  // Single-byte messages (real-time, tune request, etc.)
                    // Real-time messages don't change state, others reset to Undefined
                    if (!msg_info->is_realtime) {
                        state = Undefined;
                    }
                    break;
                    
                case 1:  // Two-byte messages (status + 1 data byte)
                    state = OneDataByte;
                    break;
                    
                case 2:  // Three-byte messages (status + 2 data bytes)
                    state = TwoDataBytes;
                    break;
                    
                case -2: // Variable length (System Exclusive)
                    state = SysEx;
                    break;
                    
                default: // Unknown or invalid (-1)
                    state = Undefined;
                    break;
            }
        } else {
            // Unknown message type - reset to undefined state
            state = Undefined;
        }
    }

    // Data bytes don't change the state - return the current one
    return state;
}

// ============================================================================
// Specialized Helper Functions
// ============================================================================

/**
 * @brief Send a single byte to the queue with thread safety
 * @param byte Pointer to the byte to send
 * @param cs Critical section for thread safety
 * @param buff Queue to send the byte to
 */
static void send_byte_to_queue(unsigned char *byte, critical_section_t *cs, queue_t *buff) {
    critical_section_enter_blocking(cs);
    queue_add_blocking(buff, byte);
    critical_section_exit(cs);
}

/**
 * @brief Send a complete multi-byte message to the queue atomically
 * @param msg Pointer to the message buffer
 * @param length Number of bytes in the message
 * @param cs Critical section for thread safety
 * @param buff Queue to send the message to
 */
static void send_complete_message(unsigned char *msg, int length, critical_section_t *cs, queue_t *buff) {
    critical_section_enter_blocking(cs);
    for (int i = 0; i < length; i++) {
        queue_add_blocking(buff, &msg[i]);
    }
    critical_section_exit(cs);
}

/**
 * @brief Check if a byte is a high-priority single-byte message
 * @param byte The byte to check
 * @return true if the byte should be sent immediately, false otherwise
 */
static bool is_immediate_message(unsigned char byte) {
    // Use lookup table for efficient checking
    const MidiMessageInfo* msg_info = lookup_midi_message(byte);
    return (msg_info != NULL) && (msg_info->data_bytes == 0);
}

/**
 * @brief Handle System Exclusive message processing
 * @param byte Pointer to the incoming byte
 * @param state Pointer to current dispatcher state
 * @param cs Critical section for thread safety
 * @param buff Queue for message output
 * @return true if the byte was processed as SysEx, false otherwise
 */
static bool handle_sysex_message(unsigned char *byte, StatusDispatcherState *state, 
                                critical_section_t *cs, queue_t *buff) {
    if ((*state == SysEx) || (*state == SysExEnd)) {
        send_byte_to_queue(byte, cs, buff);
        return true;
    }
    return false;
}

/**
 * @brief Handle data byte processing for multi-byte messages
 * @param byte Pointer to the incoming data byte
 * @param state Pointer to current dispatcher state
 * @param msg_buffer Message assembly buffer
 * @param msg_index Pointer to current position in message buffer
 * @param expected_bytes Number of expected bytes for current message
 * @param cs Critical section for thread safety
 * @param buff Queue for message output
 * @return true if a complete message was assembled and sent, false otherwise
 */
static bool handle_data_byte(unsigned char *byte, StatusDispatcherState *state,
                            unsigned char *msg_buffer, int *msg_index, int expected_bytes,
                            critical_section_t *cs, queue_t *buff) {
    // Input validation and bounds checking
    if (!byte || !state || !msg_buffer || !msg_index || !cs || !buff) {
        return false;
    }
    
    // Only process data bytes in appropriate states
    if (*state != OneDataByte && *state != TwoDataBytes) {
        return false;
    }
    
    // Bounds checking - prevent buffer overflow
    if (*msg_index >= 3) {
        *msg_index = 0;  // Reset on overflow
        return false;
    }
    
    // Store byte in assembly buffer
    msg_buffer[*msg_index] = *byte;
    (*msg_index)++;
    
    // Check if message is complete
    int required_bytes = (*state == OneDataByte) ? 2 : 3;
    if (*msg_index >= required_bytes) {
        // Send complete message atomically
        send_complete_message(msg_buffer, required_bytes, cs, buff);
        *msg_index = 0;  // Reset for next message
        return true;
    }
    
    return false;
}

/**
 * @brief Reset message assembly state
 * @param state Pointer to dispatcher state
 * @param msg_index Pointer to message buffer index
 */
static void reset_message_state(StatusDispatcherState *state, int *msg_index) {
    *msg_index = 0;
}

/**
 * @brief Main MIDI Status Dispatcher Function
 * 
 * Processes incoming MIDI bytes and dispatches complete messages to the output queue.
 * Handles different MIDI message types according to their specific requirements:
 * 
 * Message Handling Strategy:
 * - System Exclusive: Pass through immediately (variable length)
 * - Real-time messages: Send immediately (highest priority, can interrupt other messages)
 * - Channel messages: Collect complete messages before sending
 * 
 * Thread Safety:
 * Uses critical sections around queue operations to ensure thread-safe access
 * to the shared message buffer.
 * 
 * @param byte Pointer to incoming MIDI byte
 * @param cs Critical section for thread-safe queue access
 * @param buff Output queue for complete MIDI messages
 * 
 * @note Uses static variables to maintain message assembly state between calls
 * @note Real-time messages can arrive at any time and are processed immediately
 */
void status_dispatcher(unsigned char *byte, critical_section_t *cs, queue_t *buff) {
    // Input validation - critical for embedded systems reliability
    if (!byte || !cs || !buff) {
        return;  // Fail safely with null pointer protection
    }
    
    static unsigned char msg_buffer[3];    // Buffer for assembling complete MIDI messages (max 3 bytes)
    static int msg_index = 0;              // Current position in message assembly buffer
    static StatusDispatcherState state = Undefined;  // Persistent dispatcher state
    
    // Determine new state based on incoming byte
    StatusDispatcherState new_state = setState(byte);

    // State change indicates new message started - reset message assembly
    // This ensures we don't mix bytes from different messages
    if (state != new_state) {
        state = new_state;
        reset_message_state(&state, &msg_index);
    }

    // Priority 1: Handle System Exclusive messages (variable length, streamed immediately)
    if (handle_sysex_message(byte, &state, cs, buff)) {
        return;
    }

    // Priority 2: Handle immediate single-byte messages (real-time and system common)
    // These messages have HIGHEST PRIORITY and can interrupt any other message
    if (is_immediate_message(*byte)) {
        send_byte_to_queue(byte, cs, buff);
        return;  // Don't process as part of multi-byte message
    }

    // Multi-byte Message Assembly
    // Collect complete messages before sending to ensure atomic delivery
    // Message format: [Status Byte][Data Byte 1][Data Byte 2 (optional)]
    
    // Priority 3: Handle multi-byte message assembly
    // Get expected data byte count for current message type
    int expected_bytes = (state == OneDataByte) ? 1 : (state == TwoDataBytes) ? 2 : 0;
    
    if (expected_bytes > 0) {
        // Process data byte and check if message is complete
        handle_data_byte(byte, &state, msg_buffer, &msg_index, expected_bytes, cs, buff);
    }
    
    // Note: Unrecognized bytes or invalid states are silently ignored
    // This provides robust handling of corrupted or incomplete MIDI streams
}

// ============================================================================
// Helper Function Implementations
// ============================================================================

/**
 * @brief Check if a byte is a MIDI status byte
 * @param byte The byte to check
 * @return true if the byte is a status byte (MSB = 1), false otherwise
 */
bool is_status_byte(unsigned char byte) {
    return (byte & 0x80) != 0;
}

/**
 * @brief Check if a byte is a MIDI real-time message
 * @param byte The byte to check
 * @return true if the byte is a real-time message (0xF8-0xFF), false otherwise
 */
bool is_realtime_message(unsigned char byte) {
    return byte >= MIDI_CLOCK;  // 0xF8 and above
}

/**
 * @brief Get the expected number of data bytes for a given status byte
 * @param status_byte The MIDI status byte
 * @return Number of expected data bytes, or -1 if unknown message type
 */
int get_expected_data_bytes(unsigned char status_byte) {
    const MidiMessageInfo* msg_info = lookup_midi_message(status_byte);
    return (msg_info != NULL) ? msg_info->data_bytes : -1;
}

/**
 * @brief Check if a byte is a System Exclusive start byte
 * @param byte The byte to check
 * @return true if the byte is 0xF0 (SysEx start), false otherwise
 */
bool is_sysex_start(unsigned char byte) {
    return byte == MIDI_SYSTEM_EXCLUSIVE;
}

/**
 * @brief Check if a byte is a System Exclusive end byte
 * @param byte The byte to check
 * @return true if the byte is 0xF7 (SysEx end), false otherwise
 */
bool is_sysex_end(unsigned char byte) {
    return byte == MIDI_SYSTEM_EXCLUSIVE_END;
}
