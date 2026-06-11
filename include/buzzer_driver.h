#ifndef BUZZER_DRIVER_H
#define BUZZER_DRIVER_H

#include <cstdint>

constexpr uint8_t PIN_LOCAL_BUZZER = 4; // Local piezo buzzer connected to GPIO pin 4

class BuzzerDriver {
public:
    BuzzerDriver() : m_lastToggleTime(0), m_buzzerState(false) {}

    /**
     * Initializes the local buzzer pin.
     */
    void init() {
        // Configure pin as an active output channel
        // e.g., pinMode(PIN_LOCAL_BUZZER, OUTPUT);
        // turnOff();
    }

    /**
     * Forces the buzzer completely silent.
     */
    void turnOff() {
        m_buzzerState = false;
        // e.g., digitalWrite(PIN_LOCAL_BUZZER, LOW);
    }

    /**
     * Pulses the local buzzer at a specific frequency without blocking code execution.
     * @param[in] currentTimeMs The current clock ticks from your hardware timer.
     * @param[in] pulseRateMs How fast the buzzer toggles on/off (e.g., 200ms for fast beeping).
     */
    void pulse(uint32_t currentTimeMs, uint32_t pulseRateMs) {
        if (currentTimeMs - m_lastToggleTime >= pulseRateMs) {
            m_lastToggleTime = currentTimeMs;
            m_buzzerState = !m_buzzerState;

            // Drive hardware pin state
            // e.g., digitalWrite(PIN_LOCAL_BUZZER, m_buzzerState ? HIGH : LOW);
        }
    }

private:
    uint32_t m_lastToggleTime;
    bool m_buzzerState;
};

#endif // BUZZER_DRIVER_H
