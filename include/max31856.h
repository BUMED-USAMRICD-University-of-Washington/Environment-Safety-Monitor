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