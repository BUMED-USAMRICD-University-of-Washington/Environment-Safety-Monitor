// Add these definitions to your existing include/max31856.h file

// --- COLD-JUNCTION REGISTER MAP ---
constexpr uint8_t MAX31856_REG_CJTO = 0x09; // Cold-Junction Temperature Offset Register

// --- FUNCTION DECLARATION ---
/**
 * Configures the cold-junction compensation offset.
 * Used if the measurement board sits near hot compressor exhaust or localized heat sources.
 * @param[in] offsetCelsius The desired offset correction in degrees Celsius (-8°C to +7.9375°C).
 * @return bool True if the configuration was successfully written and verified.
 */
bool setColdJunctionOffset(float offsetCelsius);

// Add these to your existing include/max31856.h file

// --- MORE REGISTER MAPS ---
constexpr uint8_t MAX31856_REG_CR0 = 0x00; // Configuration 0 Register
constexpr uint8_t MAX31856_REG_CR1 = 0x01; // Configuration 1 Register

// --- CR0 BIT CONFIGURATION ---
// Bit 7: Automatic Conversion Mode (0 = Manual, 1 = Continuous)
constexpr uint8_t CR0_AUTOCONVERT_ENABLE = 0x80; 
// Bit 6: One-Shot (0 = Disabled)
// Bit 5: Open-Circuit Fault Detection (01 = Accelerated, down to 2ms)
constexpr uint8_t CR0_OCFAULT_ACCEL      = 0x10;
// Bit 4: Cold-Junction Disable (0 = Enabled)
// Bit 3: Fault Mode (0 = Interrupt, 1 = Comparator) - Comparator auto-resets when hazard clears
constexpr uint8_t CR0_FAULT_COMPARATOR   = 0x08;
// Bit 2: Fault Status Clear (0 = Normal)
// Bit 1: Notch Filter Frequency (0 = 60Hz [North America], 1 = 50Hz)
constexpr uint8_t CR0_FILTER_60HZ        = 0x00; 

// Combined CR0 Setting: Continuous conversion, accelerated fault check, comparator mode, 60Hz filter
constexpr uint8_t CR0_INIT_VALUE = (CR0_AUTOCONVERT_ENABLE | CR0_OCFAULT_ACCEL | CR0_FAULT_COMPARATOR | CR0_FILTER_60HZ);

// --- CR1 BIT CONFIGURATION (Thermocouple Type) ---
// Bits 6-4: Averaging Samples (000 = 1 sample, 001 = 2 samples, etc. Let's use 000 for maximum speed)
constexpr uint8_t CR1_AVG_1_SAMPLE = 0x00;
// Bits 3-0: Thermocouple Type (Type T is binary 0011 -> 0x03)
constexpr uint8_t CR1_TYPE_T       = 0x03;

// Combined CR1 Setting
constexpr uint8_t CR1_INIT_VALUE = (CR1_AVG_1_SAMPLE | CR1_TYPE_T);

// --- FUNCTION DECLARATION ---
/**
 * Configures the MAX31856 for Type-T thermocouples and automatic continuous testing.
 * @return bool True if configuration was successfully written.
 */
bool initMax31856();
#ifndef MAX31856_H

#define MAX31856_H

#include <cstdint>

// Register Maps
constexpr uint8_t MAX31856_REG_LTCBH = 0x0C; 
constexpr uint8_t MAX31856_REG_LTCBM = 0x0D; 
constexpr uint8_t MAX31856_REG_LTCBL = 0x0E; 
constexpr uint8_t MAX31856_REG_SR    = 0x0F; 

// Hardware Pin Assignment
constexpr uint8_t PIN_SPI_CS = 10; 

// Hardware Abstraction Hooks
void setChipSelect(uint8_t pin, bool state);
uint8_t spiTransferByte(uint8_t data);

// The Core Read Function
bool readCryoTemperature(float& temperatureCelsius);

#endif // MAX31856_H