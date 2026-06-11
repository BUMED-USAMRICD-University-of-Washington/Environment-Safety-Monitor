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
