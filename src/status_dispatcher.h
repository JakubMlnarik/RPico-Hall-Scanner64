/**
 * @file status_dispatcher.h
 * @brief MIDI Status Byte Dispatcher Module Header
 * 
 * This module provides MIDI message parsing and dispatching functionality.
 * It implements a state machine that properly handles different MIDI message
 * types according to the MIDI 1.0 specification.
 * 
 * Key Features:
 * - Real-time message priority handling
 * - System Exclusive message streaming
 * - Complete message assembly for channel messages
 * - Thread-safe queue operations
 * 
 * @author Jakub Mlnarik
 * @date 2025
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "pico/util/queue.h"
#include "pico/critical_section.h"

/**
 * @brief Maximum MIDI message buffer size
 * Should be at least 3 for standard MIDI messages.
 * Can be increased for extended MIDI protocols if needed.
 */
#define MIDI_MAX_MESSAGE_SIZE 3

/**
 * @brief Enable running status optimization (MIDI 1.0 feature)
 * Allows channel messages to omit repeated status bytes.
 * Disable if strict message framing is required.
 */
#define MIDI_RUNNING_STATUS_ENABLED 1

/**
 * @brief MIDI Message Type Constants
 * 
 * Defines all MIDI message types according to the MIDI 1.0 specification.
 * Status bytes are organized by category:
 * - 0x80-0xEF: Channel messages (with channel number in lower 4 bits)
 * - 0xF0-0xF7: System Common messages
 * - 0xF8-0xFF: System Real-time messages (highest priority)
 */
typedef enum {
    // Invalid/Error States
    MIDI_INVALID_TYPE           = 0x00,    ///< Error indicator - not a valid MIDI status
    
    // Channel Messages (0x80-0xEF) - Include channel number in lower 4 bits
    MIDI_NOTE_OFF               = 0x80,    ///< Note Off: [status][note][velocity]
    MIDI_NOTE_ON                = 0x90,    ///< Note On: [status][note][velocity]
    MIDI_AFTERTOUCH_POLY        = 0xA0,    ///< Polyphonic Key Pressure: [status][note][pressure]
    MIDI_CONTROL_CHANGE         = 0xB0,    ///< Control Change: [status][controller][value]
    MIDI_PROGRAM_CHANGE         = 0xC0,    ///< Program Change: [status][program]
    MIDI_AFTERTOUCH_CHANNEL     = 0xD0,    ///< Channel Pressure: [status][pressure]
    MIDI_PITCH_BEND             = 0xE0,    ///< Pitch Bend: [status][LSB][MSB]
    
    // System Common Messages (0xF0-0xF7)
    MIDI_SYSTEM_EXCLUSIVE       = 0xF0,    ///< System Exclusive Start (variable length)
    MIDI_SYSTEM_EXCLUSIVE_START = MIDI_SYSTEM_EXCLUSIVE,   ///< Alias for SysEx start
    MIDI_TIME_CODE_QUARTER      = 0xF1,    ///< MIDI Time Code Quarter Frame: [F1][data]
    MIDI_SONG_POSITION          = 0xF2,    ///< Song Position Pointer: [F2][LSB][MSB]
    MIDI_SONG_SELECT            = 0xF3,    ///< Song Select: [F3][song]
    MIDI_UNDEFINED_F4           = 0xF4,    ///< Reserved/Undefined
    MIDI_UNDEFINED_F5           = 0xF5,    ///< Reserved/Undefined
    MIDI_TUNE_REQUEST           = 0xF6,    ///< Tune Request (single byte)
    MIDI_SYSTEM_EXCLUSIVE_END   = 0xF7,    ///< System Exclusive End
    
    // System Real-time Messages (0xF8-0xFF) - Highest Priority, Single Byte
    MIDI_CLOCK                  = 0xF8,    ///< Timing Clock (24 per quarter note)
    MIDI_UNDEFINED_F9           = 0xF9,    ///< Reserved/Undefined
    MIDI_TICK                   = MIDI_UNDEFINED_F9, ///< Timing Tick (10ms intervals)
    MIDI_START                  = 0xFA,    ///< Start sequence
    MIDI_CONTINUE               = 0xFB,    ///< Continue sequence
    MIDI_STOP                   = 0xFC,    ///< Stop sequence
    MIDI_UNDEFINED_FD           = 0xFD,    ///< Reserved/Undefined
    MIDI_ACTIVE_SENSING         = 0xFE,    ///< Active Sensing (sent every 300ms max)
    MIDI_SYSTEM_RESET           = 0xFF,    ///< System Reset
} MidiMessageType;

/**
 * @brief Helper function declarations for MIDI message processing
 */

/**
 * @brief Check if a byte is a MIDI status byte
 * @param byte The byte to check
 * @return true if the byte is a status byte (MSB = 1), false otherwise
 */
bool is_status_byte(unsigned char byte);

/**
 * @brief Check if a byte is a MIDI real-time message
 * @param byte The byte to check
 * @return true if the byte is a real-time message (0xF8-0xFF), false otherwise
 */
bool is_realtime_message(unsigned char byte);

/**
 * @brief Get the expected number of data bytes for a given status byte
 * @param status_byte The MIDI status byte
 * @return Number of expected data bytes, or -1 if unknown message type
 */
int get_expected_data_bytes(unsigned char status_byte);

/**
 * @brief Check if a byte is a System Exclusive start byte
 * @param byte The byte to check
 * @return true if the byte is 0xF0 (SysEx start), false otherwise
 */
bool is_sysex_start(unsigned char byte);

/**
 * @brief Check if a byte is a System Exclusive end byte
 * @param byte The byte to check
 * @return true if the byte is 0xF7 (SysEx end), false otherwise
 */
bool is_sysex_end(unsigned char byte);

/**
 * @brief Main MIDI Status Dispatcher Function
 * 
 * Processes incoming MIDI bytes and dispatches complete messages to the output queue.
 * Implements a state machine to handle different MIDI message formats correctly.
 * 
 * Message Processing Strategy:
 * - System Exclusive: Stream bytes immediately (variable length)
 * - Real-time messages: Send immediately (can interrupt other messages)
 * - Channel messages: Assemble complete messages before dispatching
 * 
 * Thread Safety:
 * All queue operations are protected by critical sections to ensure
 * thread-safe access in multi-core environments.
 * 
 * @param byte Pointer to incoming MIDI byte to process
 * @param cs Critical section mutex for thread-safe queue access
 * @param buff Output queue for dispatched MIDI messages
 * 
 * @note This function uses static variables to maintain state between calls
 * @note Real-time messages (0xF8-0xFF) can arrive at any time and take priority
 * @note System Exclusive messages are streamed byte-by-byte as they arrive
 * 
 * @see MIDI 1.0 Detailed Specification for message format details
 */
extern void status_dispatcher(unsigned char *byte, critical_section_t *cs, queue_t *buff);