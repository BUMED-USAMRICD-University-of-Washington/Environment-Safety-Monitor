ifndef BUZZER_DRIVER_H
define BUZZER_DRIVER_H
include <cstdint>

class BuzzerDriver {
public:
    BuzzerDriver() : m_lastToggleTime(0), m_toggleState(false) {}
    void init() {}
    void turnOff() {}
    
    void pulse(uint32_t currentTimeMs, uint32_t intervalMs) {
        if (currentTimeMs - m_lastToggleTime < intervalMs) return;
        m_lastToggleTime = currentTimeMs;
        m_toggleState = !m_toggleState;
    }

private:
    uint32_t m_lastToggleTime;
    bool m_toggleState;
};
endif
