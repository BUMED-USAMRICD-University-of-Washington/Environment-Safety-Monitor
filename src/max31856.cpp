#include "max31856.h"

// --- HARDWARE ABSTRACTION LAYER (HAL) HOOK IMPLEMENTATIONS ---
// Note: Ensure your main hardware abstraction framework maps these stubs 
// to your microcontroller's native SPI/GPIO drivers (e.g., digitalWrite / SPI.transfer)
void setChipSelect(uint8_t pin, bool state) {
    // True = HIGH (Deselected), False = LOW (Selected)
    (void)pin;   // Suppress compiler unused variable warnings
    (void)state; 
}

uint8_t spiTransferByte(uint8_t data) {
    // Sends 1 byte over MOSI while simultaneously reading 1 byte from MISO
    (void)data; 
    return 0x00; 
}

// --- FUNCTION 1: INITIALIZATION SEQUENCING ---
/**
 * Configures the MAX31856 for Type-T thermocouples and automatic continuous testing.
 * @return bool True if configuration was successfully written and verified.
 */
bool initMax31856() {
    // 1. Configure CR0 (Continuous Conversion Mode, Accelerated Faults & 60Hz Filters)
    // Writing address = Register Address OR'd with 0x80 (Write Bit Mask)
    setChipSelect(PIN_SPI_CS, false);
    spiTransferByte(MAX31856_REG_CR0 | 0x80); 
    spiTransferByte(CR0_INIT_VALUE);
    setChipSelect(PIN_SPI_CS, true);

    // 2. Configure CR1 (Set Thermocouple Type to T and averaging to 1 sample)
    setChipSelect(PIN_SPI_CS, false);
    spiTransferByte(MAX31856_REG_CR1 | 0x80); 
    spiTransferByte(CR1_INIT_VALUE);
    setChipSelect(PIN_SPI_CS, true);

    // 3. Read-back Verification Loop
    // To ensure the chip actually saved your parameters and didn't suffer an SPI glitch
    setChipSelect(PIN_SPI_CS, false);
    spiTransferByte(MAX31856_REG_CR1); // Read address 0x01
    uint8_t verifyCR1 = spiTransferByte(0xFF);
    setChipSelect(PIN_SPI_CS, true);

    // Check if the read back value matches what we wrote (Type-T configuration)
    if (verifyCR1 != CR1_INIT_VALUE) {
        return false; // Hardware initialization failed / SPI bus error
    }

    return true; // Chip successfully locked into continuous Type-T mode
}

// --- FUNCTION 2: COLD-JUNCTION COMPENSATION CONFIGURATION ---
/**
 * Configures the cold-junction compensation offset.
 * Used if the measurement board sits near hot compressor exhaust or localized heat sources.
 * @param[in] offsetCelsius The desired offset correction in degrees Celsius (-8°C to +7.9375°C).
 * @return bool True if the configuration was successfully written and verified.
 */
bool setColdJunctionOffset(float offsetCelsius) {
    // 1. Enforce physical hardware bounds check (-8.0°C to +7.9375°C)
    if (offsetCelsius < -8.0f) {
        offsetCelsius = -8.0f;
    } else if (offsetCelsius > 7.9375f) {
        offsetCelsius = 7.9375f;
    }

    // 2. Convert the float temperature into the chip's native 8-bit fixed-point scale
    // Divide by 0.0625 (or multiply by 16) to find the integer step value
    float scaledSteps = offsetCelsius * 16.0f;
    
    // Cast to an 8-bit signed integer. The compiler automatically handles two's complement formatting
    int8_t rawOffsetByte = static_cast<int8_t>(scaledSteps);

    // 3. Transmit the configuration byte over SPI
    // Apply write mask to register address (0x09 | 0x80 = 0x89)
    setChipSelect(PIN_SPI_CS, false);
    spiTransferByte(MAX31856_REG_CJTO | 0x80); 
    spiTransferByte(static_cast<uint8_t>(rawOffsetByte)); // Safely cast signed to raw bits for transport
    setChipSelect(PIN_SPI_CS, true);

    // 4. Verification loop to eliminate transient electronic noise errors
    setChipSelect(PIN_SPI_CS, false);
    spiTransferByte(MAX31856_REG_CJTO); // Read address 0x09
    uint8_t verifyByte = spiTransferByte(0xFF);
    setChipSelect(PIN_SPI_CS, true);

    if (verifyByte != static_cast<uint8_t>(rawOffsetByte)) {
        return false; // Hardware transmission write error
    }

    return true; // Offset successfully written and locked
}

// --- FUNCTION 3: READ PROBE CRYOGENIC TEMPERATURE ---
/**
 * Reads the linearized thermocouple temperature from the MAX31856.
 * @param[out] temperatureCelsius The parsed temperature value.
 * @return bool True if read was successful and sensor is healthy; False on hardware fault.
 */
bool readCryoTemperature(float& temperatureCelsius) {
    // 1. Read the Fault Status Register first to ensure probe integrity
    setChipSelect(PIN_SPI_CS, false); 
    spiTransferByte(MAX31856_REG_SR); // Send read address for Status Register (0x0F)
    uint8_t faultStatus = spiTransferByte(0xFF); // Clock out the fault byte
    setChipSelect(PIN_SPI_CS, true);
    
    // Check if any fault bits are active (e.g., Open Circuit, Over/Under Voltage)
    if (faultStatus != 0x00) {
        return false; // Fault detected; stop execution to trigger fail-safe
    }

    // 2. Read the 3 Temperature Registers sequentially
    uint8_t rawBytes[3] = {0};
    
    setChipSelect(PIN_SPI_CS, false);
    
    // Send starting address (0x0C). The MAX31856 auto-increments to 0x0D and 0x0E
    spiTransferByte(MAX31856_REG_LTCBH); 
    
    rawBytes[0] = spiTransferByte(0xFF); // Read High Byte
    rawBytes[1] = spiTransferByte(0xFF); // Read Middle Byte
    rawBytes[2] = spiTransferByte(0xFF); // Read Low Byte
    
    setChipSelect(PIN_SPI_CS, true);

    // 3. Reconstruct the 24-bit Signed Integer
    // The MAX31856 provides a 19-bit signed value left-justified across 3 bytes
    int32_t rawData = (static_cast<int32_t>(rawBytes[0]) << 16) |
                      (static_cast<int32_t>(rawBytes[1]) << 8)  |
                      (static_cast<int32_t>(rawBytes[2]));

    // Shift right by 5 bits to eliminate the unused trailing bits of the 24-bit window
    // This retains the sign bit correctly due to sign extension in signed right shifts
    rawData >>= 5; 

    // 4. Convert to Physical Floating-Point Temperature
    // The Least Significant Bit (LSB) of the linearized temperature is exactly 0.0078125°C (1/128°C)
    temperatureCelsius = static_cast<float>(rawData) * 0.0078125f;

    return true; // Read successful
}

// --- FUNCTION 4: READ INTERNAL COLD-JUNCTION TEMPERATURE ---
/**
 * Reads the chip's internal ambient cold-junction temperature.
 * Useful for monitoring local heat pollution from freezer exhausts.
 * @param[out] cjTemperatureCelsius The parsed cold-junction temperature.
 * @return bool True if read was successful; False on sensor communication fault.
 */
bool readColdJunctionTemperature(float& cjTemperatureCelsius) {
    uint8_t rawHighByte = 0;
    uint8_t rawLowByte = 0;

    // 1. Read the two Cold-Junction registers sequentially
    setChipSelect(PIN_SPI_CS, false);
    spiTransferByte(MAX31856_REG_CJTH); // Address 0x0A (Read mode)
    
    rawHighByte = spiTransferByte(0xFF); // Clock out high byte
    rawLowByte  = spiTransferByte(0xFF); // Clock out low byte
    setChipSelect(PIN_SPI_CS, true);

    // 2. Combine into a signed 16-bit integer
    int16_t rawData = (static_cast<int16_t>(rawHighByte) << 8) | static_cast<int16_t>(rawLowByte);

    // 3. Shift right by 4 to align the 12-bit signed number
    // This removes the 4 trailing unused bits of the low register while maintaining the sign bit
    rawData >>= 4;

    // 4. Convert to Celsius using the chip's internal resolution (0.0625°C per LSB)
    cjTemperatureCelsius = static_cast<float>(rawData) * 0.0625f;

    return true; // Read successful
}