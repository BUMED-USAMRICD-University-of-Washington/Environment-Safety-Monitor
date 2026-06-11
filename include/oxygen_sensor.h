ifndef OXYGEN_SENSOR_H
define OXYGEN_SENSOR_H
include <cstdint>

constexpr uint8_t PIN_O2_ADC              = 14; 
constexpr float SYSTEM_VCC                = 5.0f;
constexpr float ADC_MAX_RESOLUTION        = 1023.0f;

constexpr float LOOP_VOLTAGE_MIN          = 1.0f;
constexpr float LOOP_VOLTAGE_MAX          = 5.0f;
constexpr float O2_PHYSICAL_MIN           = 0.0f;
constexpr float O2_PHYSICAL_MAX           = 25.0f;

bool readOxygenLevel(float& o2Percentage);

endif
