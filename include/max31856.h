ifndef MAX31856_H
define MAX31856_H
include <cstdint>

constexpr uint8_t MAX31856_REG_CR0   = 0x00;
constexpr uint8_t MAX31856_REG_CR1   = 0x01;
constexpr uint8_t MAX31856_REG_CJTO  = 0x09;
constexpr uint8_t MAX31856_REG_CJTH  = 0x0A;
constexpr uint8_t MAX31856_REG_CJTL  = 0x0B;
constexpr uint8_t MAX31856_REG_LTCBH = 0x0C;
constexpr uint8_t MAX31856_REG_LTCBM = 0x0D;
constexpr uint8_t MAX31856_REG_LTCBL = 0x0E;
constexpr uint8_t MAX31856_REG_SR    = 0x0F;

constexpr uint8_t CR0_AUTOCONVERT_ENABLE = 0x80;
constexpr uint8_t CR0_OCFAULT_ACCEL      = 0x10;
constexpr uint8_t CR0_FAULT_COMPARATOR   = 0x08;
constexpr uint8_t CR0_FILTER_60HZ        = 0x00;
constexpr uint8_t CR0_INIT_VALUE = (CR0_AUTOCONVERT_ENABLE | CR0_OCFAULT_ACCEL | CR0_FAULT_COMPARATOR | CR0_FILTER_60HZ);

constexpr uint8_t CR1_AVG_1_SAMPLE = 0x00;
constexpr uint8_t CR1_TYPE_T       = 0x03;
constexpr uint8_t CR1_INIT_VALUE = (CR1_AVG_1_SAMPLE | CR1_TYPE_T);

constexpr uint8_t PIN_SPI_CS = 10;

void setChipSelect(uint8_t pin, bool state);
uint8_t spiTransferByte(uint8_t data);

bool initMax31856();
bool setColdJunctionOffset(float offsetCelsius);
bool readColdJunctionTemperature(float& cjTemperatureCelsius);
bool readCryoTemperature(float& temperatureCelsius);

endif
