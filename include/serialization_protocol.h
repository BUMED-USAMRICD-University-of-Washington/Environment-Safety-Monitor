pragma once
include <stdint.h>
include "telemetry_types.h"

namespace SafetySystem {
    constexpr uint8_t FRAME_START_BYTE = 0x02;
    constexpr uint8_t FRAME_END_BYTE   = 0x03;

    struct __attribute__((packed)) EdwardsBulkFrameHeader {
        uint8_t  start_byte;
        uint16_t total_payload_bytes;
        uint32_t checksum_crc32;
    };
}
