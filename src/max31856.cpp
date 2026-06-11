include "max31856.h"

void setChipSelect(uint8_t pin, bool state) {
    (void)pin;   
    (void)state; 
}

uint8_t spiTransferByte(uint8_t data) {
    (void)data; 
    return 0x00; 
}

bool initMax31856() {
    setChipSelect(PIN_SPI_CS, false);
    spiTransferByte(MAX31856_REG_CR0 | 0x80); 
    spiTransferByte(CR0_INIT_VALUE);
    setChipSelect(PIN_SPI_CS, true);

    setChipSelect(PIN_SPI_CS, false);
    spiTransferByte(MAX31856_REG_CR1 | 0x80); 
    spiTransferByte(CR1_INIT_VALUE);
    setChipSelect(PIN_SPI_CS, true);

    setChipSelect(PIN_SPI_CS, false);
    spiTransferByte(MAX31856_REG_CR1); 
    uint8_t verifyCR1 = spiTransferByte(0xFF);
    setChipSelect(PIN_SPI_CS, true);

    if (verifyCR1 != CR1_INIT_VALUE) return false; 
    return true; 
}

bool setColdJunctionOffset(float offsetCelsius) {
    if (offsetCelsius < -8.0f) offsetCelsius = -8.0f;
    if (offsetCelsius > 7.9375f) offsetCelsius = 7.9375f;

    float scaledSteps = offsetCelsius * 16.0f;
    int8_t rawOffsetByte = static_cast<int8_t>(scaledSteps);

    setChipSelect(PIN_SPI_CS, false);
    spiTransferByte(MAX31856_REG_CJTO | 0x80); 
    spiTransferByte(static_cast<uint8_t>(rawOffsetByte)); 
    setChipSelect(PIN_SPI_CS, true);

    setChipSelect(PIN_SPI_CS, false);
    spiTransferByte(MAX31856_REG_CJTO); 
    uint8_t verifyByte = spiTransferByte(0xFF);
    setChipSelect(PIN_SPI_CS, true);

    if (verifyByte != static_cast<uint8_t>(rawOffsetByte)) return false; 
    return true; 
}

bool readColdJunctionTemperature(float& cjTemperatureCelsius) {
    uint8_t rawHighByte = 0;
    uint8_t rawLowByte = 0;

    setChipSelect(PIN_SPI_CS, false);
    spiTransferByte(MAX31856_REG_CJTH);
    rawHighByte = spiTransferByte(0xFF);
    rawLowByte  = spiTransferByte(0xFF);
    setChipSelect(PIN_SPI_CS, true);

    int16_t rawData = (static_cast<int16_t>(rawHighByte) << 8) | static_cast<int16_t>(rawLowByte);
    rawData >>= 4;
    cjTemperatureCelsius = static_cast<float>(rawData) * 0.0625f;

    return true;
}

bool readCryoTemperature(float& temperatureCelsius) {
    setChipSelect(PIN_SPI_CS, false);
    spiTransferByte(MAX31856_REG_SR);
    uint8_t faultStatus = spiTransferByte(0xFF);
    setChipSelect(PIN_SPI_CS, true);

    if (faultStatus != 0x00) return false;

    uint8_t rawBytes[3] = {0};
    setChipSelect(PIN_SPI_CS, false);
    spiTransferByte(MAX31856_REG_LTCBH);
    rawBytes[0] = spiTransferByte(0xFF);
    rawBytes[1] = spiTransferByte(0xFF);
    rawBytes[2] = spiTransferByte(0xFF);
    setChipSelect(PIN_SPI_CS, true);

    int32_t rawData = (static_cast<int32_t>(rawBytes[0]) << 16) |
                      (static_cast<int32_t>(rawBytes[1]) << 8)  |
                      (static_cast<int32_t>(rawBytes[2]));
    rawData >>= 5;
    temperatureCelsius = static_cast<float>(rawData) * 0.0078125f;

    return true;
}
