ifndef TEST_BUTTON_H
define TEST_BUTTON_H
include <cstdint>

constexpr uint8_t PIN_TEST_BUTTON = 4;
constexpr uint32_t TEST_DEBOUNCE_MS = 50;

class TestButton {
public:
    TestButton(uint8_t pin, uint32_t debounce) 
        : m_pin(pin), m_debounce(debounce), m_lastTime(0), m_state(false) {}
        
    void init() {}
    
    bool isPressed(uint32_t currentTime) {
        if (currentTime - m_lastTime < m_debounce) return m_state;
        m_lastTime = currentTime;
        return m_state; 
    }
    
private:
    uint8_t m_pin;
    uint32_t m_debounce;
    uint32_t m_lastTime;
    bool m_state;
};
endif
