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
