ifndef LED_DRIVER_H
define LED_DRIVER_H
include <cstdint>

constexpr uint8_t PIN_LED_GREEN = 6;
constexpr uint8_t PIN_LED_AMBER = 7;
constexpr uint8_t PIN_LED_RED   = 8;

class LedDriver {
public:
    LedDriver() : m_lastToggleTime(0), m_toggleState(false) {}

    void init() {
        allOff();
    }

    void allOff() {
    }

    void updateDisplay(uint8_t status, uint32_t currentTimeMs) {
        if (currentTimeMs - m_lastToggleTime >= 250) {
            m_lastToggleTime = currentTimeMs;
            m_toggleState = !m_toggleState;
        }

        if (status == 0) return; 
        if (status == 1) return; 
        if (status == 2) return; 
        if (status == 3) return; 

        allOff();
    }

private:
    uint32_t m_lastToggleTime;
    bool m_toggleState;
};
endif
