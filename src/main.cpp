// Add these configuration updates to your src/main.cpp file

// --- HARDWARE RELAY CONFIGURATION ---
constexpr uint8_t PIN_EDWARDS_RELAY = 3; // Connected to the Edwards Monitor Module input

enum class SafetyStatus : uint8_t { 
    SAFE, 
    WARNING, 
    CRITICAL, 
    SENSOR_FAULT 
};

/**
 * Initializes the life-safety output relay configuration.
 */
void initEdwardsInterface() {
    // 1. Force the output pin state to HIGH *before* setting it as an active OUTPUT.
    // This stops the system from accidentally tripping the Edwards loop for a split-second on boot-up.
    // e.g., digitalWrite(PIN_EDWARDS_RELAY, HIGH);
    
    // 2. Define pin as an output channel
    // e.g., pinMode(PIN_EDWARDS_RELAY, OUTPUT);
}

/**
 * Directly drives the physical circuit connected to the Edwards FireWorks ecosystem.
 */
void driveEdwardsInterface(SafetyStatus status) {
    if (status == SafetyStatus::SAFE) {
        // ENVIROMENT SAFE: Consistently energize the relay to maintain a closed circuit.
        // If the micro freezes or power dies, this automatically drops to zero, tripping the alarm.
        // e.g., digitalWrite(PIN_EDWARDS_RELAY, HIGH);
    } 
    else if (status == SafetyStatus::CRITICAL || status == SafetyStatus::SENSOR_FAULT) {
        // HAZARD ENCOUNTERED: Cut power to the relay immediately.
        // This opens the dry contacts, triggering an instant priority incident response on the FireWorks terminal.
        // e.g., digitalWrite(PIN_EDWARDS_RELAY, LOW);
    }
    // Note: We skip SafetyStatus::WARNING here so minor sensor drift doesn't force a building evacuation.
}


#include <iostream>
#include "max31856.h"
#include "oxygen_sensor.h"
#include "moving_average.h"

constexpr float O2_CRITICAL_THRESHOLD = 19.5f; // OSHA Evacuation Threshold
constexpr uint32_t SAMPLE_INTERVAL_MS = 100;

// Filter C: Smooth oxygen readings over 15 samples to clear out transient fan electrical spikes
MovingAverage<float, 15> o2Filter;

int main() {
    // Standard system boot configurations...
    if (!initMax31856()) { /* handle critical boot halt */ }

    uint32_t lastSampleTime = 0;

    while (true) {
        uint32_t currentTime = 0; // Replace with your hardware timer: e.g., millis()

        if (currentTime - lastSampleTime >= SAMPLE_INTERVAL_MS) {
            lastSampleTime = currentTime;

            float rawO2 = 0.0f;
            bool o2LoopHealthy = readOxygenLevel(rawO2);

            // Catch a broken wire or dead sensor immediately
            if (!o2LoopHealthy) {
                std::cout << "[CRITICAL FAULT] Oxygen loop broken or disconnected!" << std::endl;
                // Force facility evacuation relay open immediately via triggerAlarms()
                continue;
            }

            // Smooth the raw ADC signal through the moving average window
            float smoothedO2 = o2Filter.filter(rawO2);

            // Act on smoothed safety data
            if (smoothedO2 <= O2_CRITICAL_THRESHOLD) {
                std::cout << "[AIR HAZARD] Oxygen levels dangerously low: " << smoothedO2 << "%!" << std::endl;
                // triggerAlarms(SafetyStatus::CRITICAL);
            }
        }
    }
    return 0;
}

#include <iostream>
#include "max31856.h"
#include "moving_average.h" // Include your new filter module!

constexpr float TEMP_CRITICAL_THRESHOLD = 0.0f;  
constexpr uint32_t SAMPLE_INTERVAL_MS  = 100;   
constexpr uint32_t TELEMETRY_INTERVAL_MS = 2000; 

// Instantiate the moving average objects globally or at the top of main()
// Filter A: Smooths floor cryogenic sensor over 10 samples (1 second of data at 100ms interval)
MovingAverage<float, 10> floorTempFilter;

// Filter B: Smooths local cold-junction exhaust sensor over 30 samples to stop terminal jitter
MovingAverage<float, 30> exhaustTempFilter;

int main() {
    if (!initMax31856()) { /* Handle boot fault */ }

    uint32_t lastSampleTime = 0;
    uint32_t lastTelemetryTime = 0;

    while (true) {
        uint32_t currentTime = 0; // Replace with your hardware timer call (e.g., millis())

        // --- FAST SAFETY LOOP (100ms) ---
        if (currentTime - lastSampleTime >= SAMPLE_INTERVAL_MS) {
            lastSampleTime = currentTime;
            
            float rawCryoTemp = 0.0f;
            if (readCryoTemperature(rawCryoTemp)) {
                // Pass raw reading through the filter
                float smoothedCryoTemp = floorTempFilter.filter(rawCryoTemp);

                // Always use the smoothed data to evaluate alarms to eliminate false positives
                if (smoothedCryoTemp <= TEMP_CRITICAL_THRESHOLD) {
                    // Sound alarms! LN2 leak confirmed.
                }
            }
        }

        // --- DIAGNOSTIC TERMINAL LOOP (2000ms) ---
        if (currentTime - lastTelemetryTime >= TELEMETRY_INTERVAL_MS) {
            lastTelemetryTime = currentTime;

            float rawBoardTemp = 0.0f;
            if (readColdJunctionTemperature(rawBoardTemp)) {
                // Pass raw board temperature through its independent filter
                float smoothedBoardTemp = exhaustTempFilter.filter(rawBoardTemp);

                // Print clean, jitter-free metrics to the console terminal
                std::cout << "[TELEMETRY] Smoothed Board Temp: " 
                          << smoothedBoardTemp << " °C (Raw: " 
                          << rawBoardTemp << " °C)" << std::endl;
            }
        }
    }
    return 0;
}

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
