#pragma once
#include <stdint.h>
#include <cmath>

namespace SafetySystem {

    // 16-bit Signed ADC scale limits (ADS1115 Single-Ended range: 0 to 32767)
    constexpr double ADC_16BIT_MAX_SCALE   = 32767.0; 
    
    // PGA Full-Scale Range setting (Modify this to match your ADC configuration register)
    // Common settings: 6.144V, 4.096V, 2.048V. We use 4.096V for 3.3V/5V divider compatibility.
    constexpr double ADC_PGA_FULL_SCALE_V  = 4.096;   
    
    // Voltage divider reference specs matching your physical motherboard layout
    constexpr double VOLTAGE_DIVIDER_VCC   = 3.30;    // Voltage applied to the loop pull-up
    constexpr double R_PULLUP_OHMS         = 10000.0; // 10kΩ precision pull-up resistor
    
    // Extreme condition markers
    constexpr double OPEN_CIRCUIT_MARKER   = 999999.9;
    constexpr double DIRECT_SHORT_MARKER   = 0.0;

    class I2CADCSupervisionEngine {
    public:
        /**
         * Converts raw signed 16-bit I2C binary steps into a precise voltage reading.
         * Accounts for PGA scaling bounds.
         */
        static inline double convert_i2c_raw_to_voltage(int16_t raw_i2c_value) {
            // Negative values are mathematically invalid for single-ended ground-referenced loops
            if (raw_i2c_value < 0) return 0.0;
            if (raw_i2c_value > 32767) raw_i2c_value = 32767; // Hardware ceiling clamp

            // Calculate precise step size: Volts per Bit
            constexpr double volts_per_bit = ADC_PGA_FULL_SCALE_V / ADC_16BIT_MAX_SCALE;
            return static_cast<double>(raw_i2c_value) * volts_per_bit;
        }

        /**
         * Evaluates line voltages into absolute loop impedance ohms.
         * Enforces strict boundary checks to keep division steps safe.
         */
        static double calculate_loop_resistance(double measured_voltage) {
            // Guard A: Intercept a direct short circuit to ground
            if (measured_voltage <= 0.002) {
                return DIRECT_SHORT_MARKER;
            }

            // Guard B: Intercept a broken field wire (voltage hits the Vcc rail limit)
            // Prevents a critical divide-by-zero bug when computing (Vcc - Vout)
            if (measured_voltage >= (VOLTAGE_DIVIDER_VCC - 0.005)) {
                return OPEN_CIRCUIT_MARKER;
            }

            // Standard Voltage Divider Inverse Transposition Equation:
            // R_loop = R_pullup * (Vout / (Vcc - Vout))
            double resistance_ohms = R_PULLUP_OHMS * (measured_voltage / (VOLTAGE_DIVIDER_VCC - measured_voltage));
            
            return resistance_ohms;
        }
    };
}
