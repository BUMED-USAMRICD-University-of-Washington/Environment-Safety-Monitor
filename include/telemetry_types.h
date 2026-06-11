pragma once
include <stdint.h>

namespace SafetySystem {
    enum AlarmBits : uint8_t {
        STATE_NORMAL          = 0,          
        FAULT_O2_WIRE_BREAK   = (1 << 0),   
        FAULT_TEMP_SPI_DISCON = (1 << 1),   
        WARN_O2_DEVIATION     = (1 << 2),   
        CRIT_O2_HYPOXIA       = (1 << 3),   
        WARN_TEMP_ELEVATED    = (1 << 4),   
        CRIT_CRYO_FAILURE     = (1 << 5),   
        FAULT_WATCHDOG_OVERRUN= (1 << 6)    
    };
}

struct __attribute__((packed)) AlarmTelemetryPacket {
    uint32_t timestamp_ms;    
    float    oxygen_level;    
    float    floor_temp;      
    uint8_t  alarm_state;     
    uint16_t loop_counter;    
};
