#include <avr/wdt.h>  // Include the AVR hardware watchdog library
#include <iostream>
#include "max31856.h"
#include "oxygen_sensor.h"

// ... keep your existing thresholds and constants ...

int main() {
    // 1. HARDWARE INITIALIZATION PHASE
    initEdwardsInterface();
    
    // Configure and enable the watchdog timer with a 2-second timeout window
    wdt_enable(WDTO_2S); 
    std::cout << "[BOOT] Hardware Watchdog Timer Enabled (2s Timeout)." << std::endl;

    if (!initMax31856()) {
        // If boot fails, don't pet the dog. Let it reset the chip continuously 
        // while holding the Edwards relay loop OPEN to flag an active system fault.
        while (true) {
            driveEdwardsInterface(SafetyStatus::SENSOR_FAULT); 
        }
    }

    uint32_t lastSampleTime = 0;

    // 2. THE RUNTIME LOOP
    while (true) {
        // KICK THE DOG: Regularly reset the hardware countdown timer at the top of every loop cycle.
        // As long as the code line is reached before 2 seconds elapse, the system continues running.
        wdt_reset(); 

        uint32_t currentTime = 0; // Replace with your native millis() or get_time() call

        // --- SECTION A: CRITICAL SAFETY EVALUATION (Every 100ms) ---
        if (currentTime - lastSampleTime >= SAMPLE_INTERVAL_MS) {
            lastSampleTime = currentTime;

            float rawO2 = 0.0f;
            float rawTemp = 0.0f;

            bool o2Healthy = readOxygenLevel(rawO2);
            bool tempHealthy = readCryoTemperature(rawTemp);

            SafetyStatus systemStatus = SafetyStatus::SAFE;
            if (!o2Healthy || !tempHealthy) {
                systemStatus = SafetyStatus::SENSOR_FAULT;
            } else if (rawO2 <= O2_CRITICAL_THRESHOLD || rawTemp <= TEMP_CRITICAL_THRESHOLD) {
                systemStatus = SafetyStatus::CRITICAL;
            }

            driveEdwardsInterface(systemStatus);
        }
        
        // Note: Do NOT put wdt_reset() inside the 2000ms diagnostic loop. 
        // If Section A crashes but Section B keeps running, you want the watchdog to trip!
    }
    return 0;
}

#include <iostream>
#include <cstdint>

// Include your custom header modules from the include/ directory
#include "max31856.h"
#include "oxygen_sensor.h"
#include "moving_average.h"

// --- MONITORING THRESHOLDS & TIMING CONSTRAINTS ---
constexpr float O2_CRITICAL_THRESHOLD    = 19.5f;   // OSHA evacuation point (%)
constexpr float TEMP_CRITICAL_THRESHOLD  = 0.0f;    // Cryogenic drop point (°C)
constexpr uint32_t SAMPLE_INTERVAL_MS    = 100;     // Fast safety loop speed (10Hz)
constexpr uint32_t TELEMETRY_INTERVAL_MS = 2000;    // Terminal print rate (0.5Hz)

// --- INTERFACES & STATES ---
constexpr uint8_t PIN_EDWARDS_RELAY      = 3;       // Relay pin to Edwards Panel

enum class SafetyStatus : uint8_t { 
    SAFE, 
    WARNING, 
    CRITICAL, 
    SENSOR_FAULT 
};

// --- INITIALIZE SMOOTHING FILTERS ---
MovingAverage<float, 10> floorTempFilter;
MovingAverage<float, 15> o2Filter;

// --- HARDWARE CONFIGURATION FUNCTIONS ---
void initEdwardsInterface() {
    // Drive high immediately to keep the Edwards loop closed and happy on startup
    // e.g., digitalWrite(PIN_EDWARDS_RELAY, HIGH);
    // e.g., pinMode(PIN_EDWARDS_RELAY, OUTPUT);
}

void driveEdwardsInterface(SafetyStatus status) {
    if (status == SafetyStatus::SAFE) {
        // Safe: Maintain closed circuit loop
        // e.g., digitalWrite(PIN_EDWARDS_RELAY, HIGH);
    } else if (status == SafetyStatus::CRITICAL || status == SafetyStatus::SENSOR_FAULT) {
        // Hazard or Broken Wire: Open circuit loop to trigger FireWorks instantly
        // e.g., digitalWrite(PIN_EDWARDS_RELAY, LOW);
    }
}

// --- MASTER RUNTIME ---
int main() {
    // 1. Core Hardware Setup Phase
    initEdwardsInterface();
    
    // Configure MAX31856 to Type-T mode
    if (!initMax31856()) {
        std::cout << "[BOOT FAULT] MAX31856 chip failed to respond!" << std::endl;
        while (true) {
            driveEdwardsInterface(SafetyStatus::SENSOR_FAULT); 
        }
    }

    // Calibrate cold junction to compensate for local hallway exhaust heat
    setColdJunctionOffset(-1.5f);

    // Track internal time ticks
    uint32_t lastSampleTime = 0;
    uint32_t lastTelemetryTime = 0;

    std::cout << "[SYSTEM READY] Environment Safety Monitor Loop Active." << std::endl;

    // 2. THE RUNTIME LOOP
    while (true) {
        // Replace with your hardware time tracking call (e.g., millis() or get_time())
        uint32_t currentTime = 0; 

        // --- SECTION A: CRITICAL SAFETY EVALUATION (Runs every 100ms) ---
        if (currentTime - lastSampleTime >= SAMPLE_INTERVAL_MS) {
            lastSampleTime = currentTime;

            float rawO2 = 0.0f;
            float rawTemp = 0.0f;

            // Fetch live data streams and track physical wire integrity
            bool o2Healthy = readOxygenLevel(rawO2);
            bool tempHealthy = readCryoTemperature(rawTemp);

            SafetyStatus systemStatus = SafetyStatus::SAFE;

            // Check if a sensor sheared or broke
            if (!o2Healthy || !tempHealthy) {
                systemStatus = SafetyStatus::SENSOR_FAULT;
            } 
            else {
                // Pass raw inputs through noise smoothing filters
                float smoothedO2 = o2Filter.filter(rawO2);
                float smoothedTemp = floorTempFilter.filter(rawTemp);

                // Check against critical environmental boundaries
                if (smoothedO2 <= O2_CRITICAL_THRESHOLD || smoothedTemp <= TEMP_CRITICAL_THRESHOLD) {
                    systemStatus = SafetyStatus::CRITICAL;
                }
            }

            // Instantly communicate status to the Edwards panel via the relay
            driveEdwardsInterface(systemStatus);
        }

        // --- SECTION B: DIAGNOSTIC TELEMETRY (Runs every 2000ms) ---
        if (currentTime - lastTelemetryTime >= TELEMETRY_INTERVAL_MS) {
            lastTelemetryTime = currentTime;

            float boardAmbientTemp = 0.0f;
            if (readColdJunctionTemperature(boardAmbientTemp)) {
                std::cout << "[STATUS] Local Board/Exhaust Temp: " << boardAmbientTemp << " °C" << std::endl;
            }
        }
    }
    
    return 0; // Standard C++ contract requirement (though loop never terminates)
}
