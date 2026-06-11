ifndef CRASH_LOGGER_H
define CRASH_LOGGER_H
include <cstdint>

constexpr uint16_t EEPROM_ADDR_CRASH_COUNT   = 0x00;
constexpr uint16_t EEPROM_ADDR_LAST_REASON   = 0x01;
constexpr uint16_t EEPROM_ADDR_LOG_START     = 0x02;

enum class ResetReason : uint8_t {
    UNKNOWN          = 0x00,
    POWER_ON_RESET   = 0x01,
    EXTERNAL_RESET   = 0x02,
    WATCHDOG_TIMEOUT = 0x03,
    BROWNOUT_RESET   = 0x04
};

ResetReason detectResetReason();
void logResetToEEPROM(ResetReason reason);
void printCrashHistory();

endif
