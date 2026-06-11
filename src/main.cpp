#include <iostream>
#include <avr/wdt.h>
#include "max31856.h"
#include "oxygen_sensor.h"
#include "moving_average.h"
#include "alarm_latch.h" // Include your new validation module!

constexpr float O2_CRITICAL_THRESHOLD   = 19.5f; 
constexpr float TEMP_CRITICAL_THRESHOLD = 0.0f;  
constexpr uint32_t SAMPLE_INTERVAL_MS   = 100;   

// --- INSTANTIATE ALARM TIMING CONFIGURATIONS ---
// O2 Latch: Requires 3000ms (3 seconds) of continuous low oxygen to trip the Edwards loop
AlarmLatch o2AlarmDelay(3000);

// Cryo Latch: Requires 0ms delay. A liquid spill is an instant evacuation requirement.
AlarmLatch cryoAlarmDelay(0);

// Sensor Fault Latch: Requires 1000ms (1 second) of broken wire reads before flagging a hardware fault
AlarmLatch hardwareFaultDelay(1000);

int main() {
    initEdwardsInterface();
    wdt_enable(WDTO_2S);
    if (!initMax31856()) { /* handle boot halt */ }

    uint32_t lastSampleTime = 0;

    while (true) {
        wdt_reset();
        
        // Replace with your native microcontroller system timer call: e.g., millis()
        uint32_t currentTime = 0; 

        // --- MAIN CRITICAL SAFETY LOOP ---
        if (currentTime - lastSampleTime >= SAMPLE_INTERVAL_MS) {
            lastSampleTime = currentTime;

            float rawO2 = 0.0f;
            float rawTemp = 0.0f;

            bool o2Healthy = readOxygenLevel(rawO2);
            bool tempHealthy = readCryoTemperature(rawTemp);

            // Step 1: Check for broken wires / hardware faults
            bool currentHardwareBroken = (!o2Healthy || !tempHealthy);
            
            // Pass the state into the 1-second fault latch to absorb transient signal errors
            bool hardwareFaultAlarmActive = hardwareFaultDelay.update(currentHardwareBroken, currentTime);

            // Step 2: Evaluate actual environmental hazards
            bool currentO2Violation = (o2Healthy && (rawO2 <= O2_CRITICAL_THRESHOLD));
            bool currentTempViolation = (tempHealthy && (rawTemp <= TEMP_CRITICAL_THRESHOLD));

            // Run the states through their independent timing parameters
            bool o2AlarmActive = o2AlarmDelay.update(currentO2Violation, currentTime);
            bool cryoAlarmActive = cryoAlarmDelay.update(currentTempViolation, currentTime);

            // Step 3: Combine verified latched flags into a single master output driver
            SafetyStatus compiledStatus = SafetyStatus::SAFE;

            if (hardwareFaultAlarmActive) {
                compiledStatus = SafetyStatus::SENSOR_FAULT;
            } 
            else if (o2AlarmActive || cryoAlarmActive) {
                compiledStatus = SafetyStatus::CRITICAL;
            }

            // Step 4: Drive your output relay to the Edwards FireWorks interface
            driveEdwardsInterface(compiledStatus);
        }
    }
    return 0;
}

#include <iostream>
#include <avr/wdt.h>
#include "max31856.h"
#include "oxygen_sensor.h"
#include "crash_logger.h" // Include your new non-volatile logger!

int main() {
    // 1. PHASE ONE: IMMEDIATE INTERFACE SAFETY SECURING
    initEdwardsInterface(); // Keep relay closed to protect Fred Hutch while we check logs

    // 2. PHASE TWO: ANALYZE SYSTEM REBOOT HISTORICS
    ResetReason bootCondition = detectResetReason();
    logResetToEEPROM(bootCondition);
    
    // Output diagnostics to your maintenance terminal console line
    printCrashHistory();

    // 3. PHASE THREE: CONFIGURE WATCHDOG TIMEOUTS
    wdt_enable(WDTO_2S); // Enable a fresh 2-second watchdog countdown

    // 4. PHASE FOUR: CHIP DRIVER CONFIGURATIONS
    if (!initMax31856()) {
        while (true) {
            driveEdwardsInterface(SafetyStatus::SENSOR_FAULT); 
            // We intentionally don't reset the watchdog here. 
            // It will trigger a reboot, incrementing the log array.
        }
    }

    uint32_t lastSampleTime = 0;

    // 5. PHASE FIVE: THE MASTER RUNTIME LOOP
    while (true) {
        wdt_reset(); // Service the watchdog countdown continuously
        
        uint32_t currentTime = 0; // Replace with your hardware timer call (e.g., millis())

        if (currentTime - lastSampleTime >= SAMPLE_INTERVAL_MS) {
            lastSampleTime = currentTime;
            // ... Execute your fast-rate O2 and Temperature sampling loops ...
        }
    }
    return 0;
}

#include "esp_task_wdt.h" // Include Espressif Task Watchdog library
#include <iostream>

// Define a 2-second watchdog timeout configuration structure
#define WDT_TIMEOUT_SECONDS 2

void initWatchdog() {
    // Initialize the Task Watchdog configuration
    esp_task_wdt_config_t twdt_config = {
        .timeout_ms = WDT_TIMEOUT_SECONDS * 1000,
        .idle_core_mask = (1 << 0), // Monitor Core 0
        .trigger_panic = true       // Force a hard hardware reboot on timeout
    };
    
    esp_task_wdt_init(&twdt_config);
    esp_task_wdt_add(NULL); // Add the current running main execution loop thread to the monitor
}

int main() {
    initEdwardsInterface();
    initWatchdog(); // Arm the watchdog
    
    std::cout << "[BOOT] ESP32 Task Watchdog armed for 2 seconds." << std::endl;

    if (!initMax31856()) {
        while (true) { driveEdwardsInterface(SafetyStatus::SENSOR_FAULT); }
    }

    uint32_t lastSampleTime = 0;

    while (true) {
        // PET THE DOG: Tell the FreeRTOS subsystem this thread is healthy and looping
        esp_task_wdt_reset();

        uint32_t currentTime = 0; // Replace with esp_timer_get_time() / 1000

        if (currentTime - lastSampleTime >= SAMPLE_INTERVAL_MS) {
            lastSampleTime = currentTime;
            // ... Execute your identical O2 and Cryo sensing logic here ...
        }
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
