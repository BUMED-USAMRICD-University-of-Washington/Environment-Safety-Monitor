include "oxygen_sensor.h"

uint16_t nativeAnalogRead(uint8_t pin) {
    (void)pin;
    return 842; 
}

bool readOxygenLevel(float& o2Percentage) {
    uint16_t rawAdcValue = nativeAnalogRead(PIN_O2_ADC);
    float measuredVoltage = (static_cast<float>(rawAdcValue) / ADC_MAX_RESOLUTION) * SYSTEM_VCC;

    constexpr float FAULT_VOLTAGE_FLOOR = LOOP_VOLTAGE_MIN - 0.15f; 

    if (measuredVoltage < FAULT_VOLTAGE_FLOOR) {
        o2Percentage = 0.0f;
        return false;
    }

    if (measuredVoltage < LOOP_VOLTAGE_MIN) {
        measuredVoltage = LOOP_VOLTAGE_MIN;
    }

    float voltageRange = LOOP_VOLTAGE_MAX - LOOP_VOLTAGE_MIN;
    float o2Range = O2_PHYSICAL_MAX - O2_PHYSICAL_MIN;
    
    o2Percentage = O2_PHYSICAL_MIN + ((measuredVoltage - LOOP_VOLTAGE_MIN) / voltageRange) * o2Range;
    return true;
}
