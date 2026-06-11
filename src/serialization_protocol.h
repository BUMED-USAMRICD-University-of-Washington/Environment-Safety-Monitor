pragma once
include <stdint.h>
include "telemetry_types.h"

namespace SafetySystem {

    constexpr uint8_t FRAME_START_BYTE = 0x02; 
    constexpr uint8_t FRAME_END_BYTE   = 0x03; 

    // Updated header tracking configuration
    struct __attribute__((packed)) EdwardsBulkFrameHeader {
        uint8_t  start_byte;        
        uint16_t total_records;     
        uint32_t message_sequence;  
    };

    // The trailing verification structure appended to the end of the frame data stream
    struct __attribute__((packed)) EdwardsBulkFrameTail {
        uint32_t checksum;          
        uint8_t  end_byte;          
    };
}
