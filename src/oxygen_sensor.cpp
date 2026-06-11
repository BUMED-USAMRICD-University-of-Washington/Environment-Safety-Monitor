#include "oxygen_sensor.h"

// Hardware Abstraction Layer stub - replace with your specific platform's ADC read
uint16_t nativeAnalogRead(uint8_t pin) {
    // e.g., return analogRead(pin); or HAL_ADC_GetValue(&hadc1);
    return 842; // Simulated safe atmospheric baseline (approx 20.9% O2)
}

bool readOxygenLevel(float& o2Percentage) {
    // 1. Gather raw step counts from the ADC register
    uint16_t rawAdcValue = nativeAnalogRead(PIN_O2_ADC);

    // 2. Convert raw steps into actual measured voltage
    float measuredVoltage = (static_cast<float>(rawAdcValue) / ADC_MAX_RESOLUTION) * SYSTEM_VCC;

    // 3. Fail-Safe Wire Break Check (Live Zero Rule)
    // In an industrial 4-20mA system, a live sensor never outputs 0 Volts. 
    // If voltage drops significantly below the 4mA floor (1.0V), a wire is broken.
    constexpr float FAULT_VOLTAGE_FLOOR = LOOP_VOLTAGE_MIN - 0.15V; // 0.85V buffer
    if (measuredVoltage < FAULT_VOLTAGE_FLOOR) {
        o2Percentage = 0.0f;
        return false; // Return false immediately to trigger SENSOR_FAULT alarm mode
    }

    // Protect math boundary against minor calibration drift slightly below 4mA
    if (measuredVoltage < LOOP_VOLTAGE_MIN) {
        measuredVoltage = LOOP_VOLTAGE_MIN;
    }

    // 4. Linear Interpolation Formula (y = mx + b translation)
    // Maps the 1.0V - 5.0V input domain to the 0.0% - 25.0% Oxygen physical domain
    float voltageRange = LOOP_VOLTAGE_MAX - LOOP_VOLTAGE_MIN;
    float o2Range = O2_PHYSICAL_MAX - O2_PHYSICAL_MIN;
    
    o2Percentage = O2_PHYSICAL_MIN + ((measuredVoltage - LOOP_VOLTAGE_MIN) / voltageRange) * o2Range;

    return true; // Loop is stable and active
}
