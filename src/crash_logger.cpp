#include "crash_logger.h"
#include <avr/eeprom.h>  // Include native microcontroller EEPROM drivers
#include <avr/io.h>      // Include hardware register definitions
#include <iostream>

ResetReason detectResetReason() {
    // Read the MCU Status Register (MCUSR) which holds the hardware boot flags
    uint8_t mcuStatus = MCUSR;
    
    // Clear the register immediately so the next reset flag can be captured cleanly
    MCUSR = 0x00;

    // Evaluate hardware flags using bitwise masking
    if (mcuStatus & (1 << WDRF)) {
        return ResetReason::WATCHDOG_TIMEOUT;
    } else if (mcuStatus & (1 << BORF)) {
        return ResetReason::BROWNOUT_RESET;
    } else if (mcuStatus & (1 << EXTRF)) {
        return ResetReason::EXTERNAL_RESET;
    } else if (mcuStatus & (1 << PORF)) {
        return ResetReason::POWER_ON_RESET;
    }
    
    return ResetReason::UNKNOWN;
}

void logResetToEEPROM(ResetReason reason) {
    // Always store the most recent reason directly to address 0x01
    eeprom_update_byte(reinterpret_cast<uint8_t*>(EEPROM_ADDR_LAST_REASON), static_cast<uint8_t>(reason));

    // If it was a genuine watchdog crash, increment your persistent fault counter
    if (reason == ResetReason::WATCHDOG_TIMEOUT) {
        uint8_t currentCrashCount = eeprom_read_byte(reinterpret_cast<uint8_t*>(EEPROM_ADDR_CRASH_COUNT));
        
        // Prevent 8-bit integer overflow (max 255)
        if (currentCrashCount < 255) {
            currentCrashCount++;
        }
        
        // eeprom_update_byte only writes if the value changed, preserving EEPROM cell lifespan
        eeprom_update_byte(reinterpret_cast<uint8_t*>(EEPROM_ADDR_CRASH_COUNT), currentCrashCount);
    }
}

void printCrashHistory() {
    uint8_t crashes = eeprom_read_byte(reinterpret_cast<uint8_t*>(EEPROM_ADDR_CRASH_COUNT));
    uint8_t lastCode = eeprom_read_byte(reinterpret_cast<uint8_t*>(EEPROM_ADDR_LAST_REASON));

    std::cout << "\n--- NON-VOLATILE FAULT TELEMETRY ---" << std::endl;
    std::cout << "[EEPROM] Cumulative Watchdog Failures: " << static_cast<int>(crashes) << std::endl;
    std::cout << "[EEPROM] Last Boot Condition Code: ";
    
    switch (static_cast<ResetReason>(lastCode)) {
        case ResetReason::POWER_ON_RESET:   std::cout << "NORMAL POWER ON" << std::endl; break;
        case ResetReason::EXTERNAL_RESET:   std::cout << "MANUAL BUTTON PRESS" << std::endl; break;
        case ResetReason::WATCHDOG_TIMEOUT: std::cout << "CRITICAL WATCHDOG TIMEOUT (SYSTEM FROZE)" << std::endl; break;
        case ResetReason::BROWNOUT_RESET:   std::cout << "VOLTAGE BROWNOUT FLUID DUMP" << std::endl; break;
        default:                            std::cout << "UNKNOWN / INITIALIZED STATE" << std::endl; break;
    }
    std::cout << "------------------------------------\n" << std::endl;
}
