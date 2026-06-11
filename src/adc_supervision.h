#pragma once
#include <stdint.h>

namespace SafetySystem {

    // Circuit hardware design constants (tweak based on your physical PCB values)
    constexpr double ADC_REF_VOLTAGE  = 3.30;     // Vcc voltage rail (3.3V typical)
    constexpr double ADC_RESOLUTION   = 4095.0;   // 12-bit ADC max scale (2^12 - 1)
    constexpr double R_PULLUP_OHMS    = 10000.0;  // 10kΩ internal pull-up resistor
    
    // Safety thresholds for extreme hardware fault validation
    constexpr double OPEN_CIRCUIT_MARKER = 999999.9; // Returned if loop is physically cut

    class ADCSupervisionEngine {
    public:
        /**
         * Converts a raw 12-bit ADC integer step into a precise voltage reading.
         */
        static inline double convert_raw_to_voltage(uint16_t raw_adc_value) {
            if (raw_adc_value > 4095) raw_adc_value = 4095; // Hard ceiling limit clamp
            return (static_cast<double>(raw_adc_value) * ADC_REF_VOLTAGE) / ADC_RESOLUTION;
        }

        /**
         * Processes an analog voltage reading to determine the exact resistance of the field loop.
         * Enforces strict safety boundary evaluations to prevent mathematical faults.
         */
        static double calculate_loop_resistance(double measured_voltage) {
            // Boundary Case A: Closed loop short circuit or near-zero voltage line
            if (measured_voltage <= 0.005) {
                return 0.0; // Dead short directly to ground
            }

            // Boundary Case B: Open circuit wire shear or near-Vcc voltage saturation
            // Prevents a critical divide-by-zero condition when calculating (Vcc - Vout)
            if (measured_voltage >= (ADC_REF_VOLTAGE - 0.005)) {
                return OPEN_CIRCUIT_MARKER; // Infinite resistance
            }

            // Standard Voltage Divider Transposition Equation
            // R_loop = R_pullup * (Vout / (Vcc - Vout))
            double resistance_ohms = R_PULLUP_OHMS * (measured_voltage / (ADC_REF_VOLTAGE - measured_voltage));
            
            return resistance_ohms;
        }
    };
}
