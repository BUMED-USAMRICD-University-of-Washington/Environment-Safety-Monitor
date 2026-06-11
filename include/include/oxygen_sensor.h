#ifndef OXYGEN_SENSOR_H
#define OXYGEN_SENSOR_H

#include <cstdint>

// --- HARDWARE CONFIGURATION ---
constexpr uint8_t PIN_O2_ADC              = A0;     // Microcontroller analog input pin
constexpr float SYSTEM_VCC                = 5.0f;   // ADC reference voltage (5.0V or 3.3V)
constexpr float ADC_MAX_RESOLUTION        = 1023.0f;// 10-bit ADC = 1023, 12-bit ADC = 4095

// --- INDUSTRIAL LOOP CALIBRATION VALUES ---
constexpr float LOOP_VOLTAGE_MIN          = 1.0f;   // Voltage corresponding to 4mA (0% O2)
constexpr float LOOP_VOLTAGE_MAX          = 5.0f;   // Voltage corresponding to 20mA (25% O2)
constexpr float O2_PHYSICAL_MIN           = 0.0f;   // 4mA output = 0.0% Oxygen
constexpr float O2_PHYSICAL_MAX           = 25.0f;  // 20mA output = 25.0% Oxygen

// --- FUNCTION DECLARATION ---
/**
 * Reads the 4-20mA Oxygen sensor via an ADC channel and translates it to an exact air percentage.
 * @param[out] o2Percentage The calculated oxygen concentration percentage (0.0% to 25.0%).
 * @return bool True if the loop is healthy; False if current drops below 4mA (broken wire / loop fault).
 */
bool readOxygenLevel(float& o2Percentage);

#endif // OXYGEN_SENSOR_H
