#pragma once
#include <stdint.h>

namespace SafetySystem {

    // Unique bit assignments representing independent hardware and environmental flags
    enum AlarmBits : uint8_t {
        STATE_NORMAL          = 0,         // 0000 0000 -> All parameters nominal
        FAULT_O2_WIRE_BREAK   = (1 << 0),  // 0000 0001 -> Live Zero error (<0.85V) on Oxygen line
        FAULT_TEMP_SPI_DISCON = (1 << 1),  // 0000 0010 -> MAX31856 connection failure / Open circuit
        WARN_O2_DEVIATION     = (1 << 2),  // 0000 0100 -> O2 dropping below 19.5% (Pre-alarm warning)
        CRIT_O2_HYPOXIA       = (1 << 3),  // 0000 1000 -> O2 dropped below 18.0% (Immediate relay trip)
        WARN_TEMP_ELEVATED    = (1 << 4),  // 0001 0000 -> Cryogenic temp rising past warning threshold
        CRIT_TEMP_THERMAL_RUN = (1 << 5),  // 0010 0000 -> Cryogenic containment breach (Immediate relay trip)
        SYSTEM_WATCHDOG_JITTER= (1 << 6)   // 0100 0000 -> 100ms loop processing delayed past safe limits
    };

    // Packed telemetry packet structure optimized for inter-core FIFO streaming
    struct __attribute__((packed)) AlarmTelemetryPacket {
        uint32_t timestamp_ms;   // Milliseconds elapsed since system initialization
        float    oxygen_level;   // Scaled environmental oxygen value
        float    floor_temp;     // Cryogenic temperature in Celsius
        uint8_t  alarm_state;    // Aggregated bitmask using the AlarmBits specifications
        uint16_t loop_counter;   // Diagnostic index to monitor packet delivery consistency
    };

    // -------------------------------------------------------------------------
    // INLINE COMPILER UTILITIES (Zero-overhead bitmask manipulation macros)
    // -------------------------------------------------------------------------
    inline void set_flag(uint8_t& mask, AlarmBits flag)   { mask |= flag; }
    inline void clear_flag(uint8_t& mask, AlarmBits flag) { mask &= ~flag; }
    inline bool check_flag(uint8_t mask, AlarmBits flag)  { return (mask & flag) != 0; }
    
    // Checks if the bitmask contains flags that require a physical trip of the NC safety relay
    inline bool requires_relay_trip(uint8_t mask) {
        constexpr uint8_t CRITICAL_TRIP_MASK = FAULT_O2_WIRE_BREAK | 
                                               FAULT_TEMP_SPI_DISCON | 
                                               CRIT_O2_HYPOXIA | 
                                               CRIT_TEMP_THERMAL_RUN | 
                                               SYSTEM_WATCHDOG_JITTER;
        return (mask & CRITICAL_TRIP_MASK) != 0;
    }
}
