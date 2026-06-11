// Update your main monitoring block in src/main.cpp
#include <iostream>
#include "max31856.h"

// Configuration timing for terminal output (Separate from fast safety sampling rate)
constexpr uint32_t TELEMETRY_INTERVAL_MS = 2000; // Output to terminal every 2 seconds

int main() {
    // Standard system initializations...
    if (!initMax31856()) { /* handle error */ }

    uint32_t lastSampleTime = 0;
    uint32_t lastTelemetryTime = 0;

    while (true) {
        uint32_t currentTime = 0; // Hook your physical hardware timer (e.g., millis()) here
        
        // --- FAST CRITICAL SAFETY LOOP ---
        if (currentTime - lastSampleTime >= SAMPLE_INTERVAL_MS) {
            lastSampleTime = currentTime;
            float currentProbeTemp = 0.0f;
            readCryoTemperature(currentProbeTemp);
            // Process critical thresholds...
        }

        // --- SLOWER DIAGNOSTIC TERMINAL LOOP ---
        if (currentTime - lastTelemetryTime >= TELEMETRY_INTERVAL_MS) {
            lastTelemetryTime = currentTime;

            float localBoardTemp = 0.0f;
            if (readColdJunctionTemperature(localBoardTemp)) {
                // Display local heat pollution in the terminal
                std::cout << "[DIAGNOSTIC] Board/Exhaust Temp: " 
                          << localBoardTemp 
                          << " °C" << std::endl;
                          
                // Safe operation envelope boundary check
                if (localBoardTemp > 85.0f) {
                    std::cout << "[WARNING] Hardware exceeding operating limits near freezer!" << std::endl;
                }
            } else {
                std::cout << "[ERROR] Failed to read internal cold-junction sensor!" << std::endl;
            }
        }
    }
    return 0;
}

// Inside your main startup routine in src/main.cpp
int main() {
    // Initialize IO pins and SPI bus...
    
    // Step A: Set to Type-T mode
    if (!initMax31856()) {
        // Handle boot fault...
    }

    // Step B: Calibrate for local heat pollution if deployed on a hot leg or exhaust zone
    // Example: Offset local hardware error by -1.5°C
    float siteSpecificOffset = -1.5f; 
    
    if (!setColdJunctionOffset(siteSpecificOffset)) {
        // If the configuration write fails, trip safety loop out to BMS
        while (true) {
            triggerAlarms(SafetyStatus::SENSOR_FAULT);
        }
    }

    // Begin infinite ambient monitoring loop safely...
    while (true) {
        // readCryoTemperature()...
    }
    return 0;
}

// Update your src/main.cpp file initialization routine
int main() {
    // 1. Initialize your physical GPIO pins and SPI clock speeds first
    // native_spi_init_hooks();

    // 2. Flash configure the MAX31856 chip to Type-T mode
    bool chipReady = initMax31856();
    
    if (!chipReady) {
        // CRITICAL BOOT FAULT: The hardware didn't respond or reject configuration.
        // Instantly trip the safety relay out to the building management system (BMS) 
        // because we cannot safely monitor the environment.
        while (true) {
            triggerAlarms(SafetyStatus::SENSOR_FAULT); 
        }
    }

    // 3. Enter infinite safety loop if initialization succeeds
    uint32_t lastSampleTime = 0;
    while (true) {
        // Continual non-blocking execution code runs here...
    }
    return 0;
}

#include <cstdint>
#include "max31856.h" // Include your new chip helper!

constexpr float O2_CRITICAL_THRESHOLD  = 19.5f; 
constexpr float TEMP_CRITICAL_THRESHOLD = 0.0f;  
constexpr uint32_t SAMPLE_INTERVAL_MS  = 100;   

enum class SafetyStatus : uint8_t { SAFE, WARNING, CRITICAL, SENSOR_FAULT };

// Note: Implement your hardware stubs here for setChipSelect and spiTransferByte

int main() {
    uint32_t lastSampleTime = 0; 
    
    while (true) {
        uint32_t currentTime = 0; // Hook your physical hardware timer here
        
        if (currentTime - lastSampleTime >= SAMPLE_INTERVAL_MS) {
            lastSampleTime = currentTime;
            
            float currentTemp = 0.0f;
            // Use the return value to capture a frozen sensor fault instantly
            bool sensorHealthy = readCryoTemperature(currentTemp);
            
            if (!sensorHealthy || currentTemp <= TEMP_CRITICAL_THRESHOLD) {
                // Sound the alarms immediately!
            }
        }
    }
    return 0;
}
