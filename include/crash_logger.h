#ifndef CRASH_LOGGER_H
#define CRASH_LOGGER_H

#include <cstdint>

// --- EEPROM MEMORY MAP ---
constexpr uint16_t EEPROM_ADDR_CRASH_COUNT   = 0x00; // Total number of watchdog reboots
constexpr uint16_t EEPROM_ADDR_LAST_REASON   = 0x01; // The reason code for the latest reset
constexpr uint16_t EEPROM_ADDR_LOG_START     = 0x02; // Start address for a simple circular log buffer

// --- RESET REASON CODES ---
enum class ResetReason : uint8_t {
    UNKNOWN          = 0x00,
    POWER_ON_RESET   = 0x01, // Normal power plugin
    EXTERNAL_RESET   = 0x02, // Manual reset button pressed
    WATCHDOG_TIMEOUT = 0x03, // Software locked up, watchdog tripped!
    BROWNOUT_RESET   = 0x04  // Power supply voltage dropped too low
};

// --- FUNCTION DECLARATIONS ---
/**
 * Detects why the system rebooted by checking internal hardware status registers.
 * @return ResetReason The evaluated cause of the microcontroller reset.
 */
ResetReason detectResetReason();

/**
 * Checks the last boot reason and logs it to non-volatile EEPROM memory if it was a fault.
 * @param[in] reason The detected reset reason.
 */
void logResetToEEPROM(ResetReason reason);

/**
 * Prints the historical crash telemetry stored in EEPROM directly to the console terminal.
 */
void printCrashHistory();

#endif // CRASH_LOGGER_H
