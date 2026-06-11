// Add this implementation to your src/max31856.cpp file
#include "max31856.h"

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

    return true; 
}

// Add this implementation to your src/max31856.cpp file
#include "max31856.h"

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
    spiTransferByte(MAX31856_REG_CJTO); // Read back mode
    uint8_t verifyByte = spiTransferByte(0xFF);
    setChipSelect(PIN_SPI_CS, true);

    if (verifyByte != static_cast<uint8_t>(rawOffsetByte)) {
        return false; // Hardware transmission write error
    }

    return true; // Offset successfully written and locked
}

// Add this implementation to your src/max31856.cpp file
#include "max31856.h"

bool initMax31856() {
    
    // 1. Configure CR0 (Continuous Conversion Mode & Filters)
    // Writing address = Register Address OR'd with 0x80 (Write Bit)
    setChipSelect(PIN_SPI_CS, false);
    spiTransferByte(MAX31856_REG_CR0 | 0x80); 
    spiTransferByte(CR0_INIT_VALUE);
    setChipSelect(PIN_SPI_CS, true);

    // 2. Configure CR1 (Set Thermocouple Type to T)
    setChipSelect(PIN_SPI_CS, false);
    spiTransferByte(MAX31856_REG_CR1 | 0x80); 
    spiTransferByte(CR1_INIT_VALUE);
    setChipSelect(PIN_SPI_CS, true);

    // 3. Optional: Read-back Verification Loop
    // To ensure the chip actually saved your parameters and didn't suffer an SPI glitch
    setChipSelect(PIN_SPI_CS, false);
    spiTransferByte(MAX31856_REG_CR1); // Read address 0x01
    uint8_t verifyCR1 = spiTransferByte(0xFF);
    setChipSelect(PIN_SPI_CS, true);

    // Check if the read back value matches what we wrote (Type-T configuration)
    if (verifyCR1 != CR1_INIT_VALUE) {
        return false; // Hardware initialization failed
    }

    return true; // Chip successfully locked into Type-T mode
}

#include "max31856.h"

bool readCryoTemperature(float& temperatureCelsius) {
    // 1. Read Fault Status Register
    setChipSelect(PIN_SPI_CS, false); 
    spiTransferByte(MAX31856_REG_SR); 
    uint8_t faultStatus = spiTransferByte(0xFF); 
    setChipSelect(PIN_SPI_CS, true);
    
    if (faultStatus != 0x00) {
        return false; 
    }

    // 2. Read Temperature Registers
    uint8_t rawBytes[3] = {0};
    
    setChipSelect(PIN_SPI_CS, false);
    spiTransferByte(MAX31856_REG_LTCBH); 
    
    rawBytes[0] = spiTransferByte(0xFF); 
    rawBytes[1] = spiTransferByte(0xFF); 
    rawBytes[2] = spiTransferByte(0xFF); 
    setChipSelect(PIN_SPI_CS, true);

    // 3. Reconstruct and sign-extend the 24-bit value
    int32_t rawData = (static_cast<int32_t>(rawBytes[0]) << 16) |
                      (static_cast<int32_t>(rawBytes[1]) << 8)  |
                      (static_cast<int32_t>(rawBytes[2]));

    rawData >>= 5; 

    // 4. Convert to Celsius
    temperatureCelsius = static_cast<float>(rawData) * 0.0078125f;
    return true; 
}
