#ifndef LED_DRIVER_H
#define LED_DRIVER_H

#include <cstdint>

// --- LED HARDWARE PIN MAPPING ---
constexpr uint8_t PIN_LED_GREEN = 6;
constexpr uint8_t PIN_LED_AMBER = 7;
constexpr uint8_t PIN_LED_RED   = 8;

class LedDriver {
public:
    LedDriver() : m_lastToggleTime(0), m_toggleState(false) {}

    void init() {
        // Set all pins as active outputs
        // e.g., pinMode(PIN_LED_GREEN, OUTPUT);
        // e.g., pinMode(PIN_LED_AMBER, OUTPUT);
        // e.g., pinMode(PIN_LED_RED, OUTPUT);
        allOff();
    }

    void allOff() {
        // e.g., digitalWrite(PIN_LED_GREEN, LOW);
        // e.g., digitalWrite(PIN_LED_AMBER, LOW);
        // e.g., digitalWrite(PIN_LED_RED, LOW);
    }

    /**
     * Drives the visual display array based on the evaluated safety status.
     * @param[in] status The compiled system status from the safety loop.
     * @param[in] currentTimeMs The current clock ticks from the hardware timer.
     */
    void updateDisplay(uint8_t status, uint32_t currentTimeMs) {
        // Handle blinking intervals
        if (currentTimeMs - m_lastToggleTime >= 250) { // 250ms blink rate
            m_lastToggleTime = currentTimeMs;
            m_toggleState = !m_toggleState;
        }

        // Cast integer back to your status definition
        switch (status) {
            case 0: // SAFE / STANDBY -> Constant Solid Green
                // e.g., digitalWrite(PIN_LED_GREEN, HIGH);
                // e.g., digitalWrite(PIN_LED_AMBER, LOW);
                // e.g., digitalWrite(PIN_LED_RED, LOW);
                break;

            case 1: // WARNING / COUNTDOWN WINDOW -> Flashing Amber
                // e.g., digitalWrite(PIN_LED_GREEN, LOW);
                // e.g., digitalWrite(PIN_LED_AMBER, m_toggleState ? HIGH : LOW);
                // e.g., digitalWrite(PIN_LED_RED, LOW);
                break;

            case 2: // CRITICAL ALARM / TEST -> Rapid Flashing Red
                // e.g., digitalWrite(PIN_LED_GREEN, LOW);
                // e.g., digitalWrite(PIN_LED_AMBER, LOW);
                // e.g., digitalWrite(PIN_LED_RED, m_toggleState ? HIGH : LOW);
                break;

            case 3: // SENSOR FAULT -> Solid Amber (Steady indicator of maintenance needs)
                // e.g., digitalWrite(PIN_LED_GREEN, LOW);
                // e.g., digitalWrite(PIN_LED_AMBER, HIGH);
                // e.g., digitalWrite(PIN_LED_RED, LOW);
                break;

            default: // Fallback safety catch
                allOff();
                break;
        }
    }

private:
    uint32_t m_lastToggleTime;
    bool m_toggleState;
};

#endif // LED_DRIVER_H