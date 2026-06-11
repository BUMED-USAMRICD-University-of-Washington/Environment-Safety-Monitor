#pragma once
#include <stdint.h>

namespace SafetySystem {

    enum AlarmBits : uint8_t {
        STATE_NORMAL          = 0,
        FAULT_O2_WIRE_BREAK   = (1 << 0),
        FAULT_TEMP_SPI_DISCON = (1 << 1),
        WARN_O2_DEVIATION     = (1 << 2),
        CRIT_O2_HYPOXIA       = (1 << 3),  // Requires Relay Trip + Core 1 Network Trigger
        WARN_TEMP_ELEVATED    = (1 << 4),
        CRIT_TEMP_THERMAL_RUN = (1 << 5),  // Requires Relay Trip + Core 1 Network Trigger
        SYSTEM_WATCHDOG_JITTER= (1 << 6)
    };

    struct __attribute__((packed)) AlarmTelemetryPacket {
        uint32_t timestamp_ms;
        float    oxygen_level;
        float    floor_temp;
        uint8_t  alarm_state;
        uint16_t loop_counter;
    };

    // Quick helper to see if Core 1 needs to spawn an asynchronous network alert dispatch thread
    inline bool requires_network_dispatch(uint8_t mask) {
        constexpr uint8_t NETWORK_DISPATCH_MASK = CRIT_O2_HYPOXIA | CRIT_TEMP_THERMAL_RUN;
        return (mask & NETWORK_DISPATCH_MASK) != 0;
    }
}
