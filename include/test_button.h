#ifndef TEST_BUTTON_H
#define TEST_BUTTON_H

#include <cstdint>

class TestButton {
public:
    TestButton(uint8_t pin, uint32_t debounceDelayMs) 
        : m_pin(pin), m_debounceDelayMs(debounceDelayMs), m_lastDebounceTime(0), m_stableState(false), m_lastRawState(false) {}

    void init() {
        // Configure pin as INPUT with an internal pull-up resistor. 
        // This means the pin reads HIGH normally, and drops to LOW when pressed.
        // e.g., pinMode(m_pin, INPUT_PULLUP);
    }

    /**
     * Reads and debounces the physical button.
     * @param[in] currentTimeMs Current clock ticks.
     * @return bool True ONLY while the button is actively being pressed down.
     */
    bool isPressed(uint32_t currentTimeMs) {
        // Read raw active-low hardware state (Invert with ! because pressing shorts it to ground)
        // e.g., bool rawRead = !digitalRead(m_pin);
        bool rawRead = false; // Stub replacement for native reading

        if (rawRead != m_lastRawState) {
            m_lastDebounceTime = currentTimeMs; // Reset debounce timer if state jitters
        }

        if ((currentTimeMs - m_lastDebounceTime) >= m_debounceDelayMs) {
            m_stableState = rawRead; // Lock in state once it remains steady for 50ms
        }

        m_lastRawState = rawRead;
        return m_stableState;
    }

private:
    uint8_t m_pin;
    uint32_t m_debounceDelayMs;
    uint32_t m_lastDebounceTime;
    bool m_stableState;
    bool m_lastRawState;
};

#endif // TEST_BUTTON_H
