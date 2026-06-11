#pragma once
#include <stdint.h>
#include "telemetry_types.h"

namespace SafetySystem {

    constexpr uint8_t FRAME_START_BYTE = 0x02; // STX
    constexpr uint8_t FRAME_END_BYTE   = 0x03; // ETX

    // Updated header tracking configuration
    struct __attribute__((packed)) EdwardsBulkFrameHeader {
        uint8_t  start_byte;       // 0x02
        uint16_t total_records;     // Quantity of outlet structures following
        uint32_t message_sequence;  // Tracking index counter
    };

    // The trailing verification structure appended to the end of the frame data stream
    struct __attribute__((packed)) EdwardsBulkFrameTail {
        uint32_t checksum;          // Calculated CRC32 value over header and payload data
        uint8_t  end_byte;          // 0x03
    };
}
