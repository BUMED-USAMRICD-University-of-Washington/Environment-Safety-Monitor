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
