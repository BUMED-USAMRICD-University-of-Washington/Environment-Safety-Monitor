#include <iostream>
#include <cstdint>
#include <avr/wdt.h>  // AVR Hardware Watchdog Library

// Include custom header modules from your project's include/ directory
#include "max31856.h"
#include "oxygen_sensor.h"
#include "moving_average.h"
#include "alarm_latch.h"
#include "buzzer_driver.h"
#include "test_button.h"
#include "crash_logger.h"
#include "led_driver.h"

// --- SYSTEM THRESHOLDS & TIMING PROFILE ---
constexpr float O2_CRITICAL_THRESHOLD    = 19.5f;   // OSHA evacuation point (%)
constexpr float TEMP_CRITICAL_THRESHOLD  = 0.0f;    // Cryogenic floor limit (°C)
constexpr uint32_t SAMPLE_INTERVAL_MS    = 100;     // Fast safety loop speed (10Hz)
constexpr uint32_t TELEMETRY_INTERVAL_MS = 2000;    // Terminal diagnostic print rate

// --- HARDWARE PIN ASSIGNMENTS ---
constexpr uint8_t PIN_EDWARDS_RELAY      = 3;       // Relay output pin to Edwards panel

// --- INTERFACE AND TIME INSTANTIATION ---
MovingAverage<float, 10> floorTempFilter;
MovingAverage<float, 15> o2Filter;

AlarmLatch o2AlarmDelay(3000);        // 3-second delay for air displacement validation
AlarmLatch cryoAlarmDelay(0);         // 0-second delay for liquid spills (instant trip)
AlarmLatch hardwareFaultDelay(1000);  // 1-second delay buffer for electrical dropouts

BuzzerDriver localBuzzer;
TestButton inspectorButton(PIN_TEST_BUTTON, TEST_DEBOUNCE_MS);
LedDriver statusVisuals;

// --- HARDWARE CONTROL LAYERS ---
void initEdwardsInterface() {
    // Drive pin HIGH immediately to secure the Edwards loop on startup
    // e.g., digitalWrite(PIN_EDWARDS_RELAY, HIGH);
    // e.g., pinMode(PIN_EDWARDS_RELAY, OUTPUT);
}

void driveEdwardsInterface(SafetyStatus status) {
    if (status == SafetyStatus::SAFE) {
        // Safe Condition: Maintain continuously energized loop
        // e.g., digitalWrite(PIN_EDWARDS_RELAY, HIGH);
    } else if (status == SafetyStatus::CRITICAL || status == SafetyStatus::SENSOR_FAULT) {
        // Hazard or Device Failure: Drop circuit power to trip Edwards FireWorks instantly
        // e.g., digitalWrite(PIN_EDWARDS_RELAY, LOW);
    }
}

// --- MASTER EXECUTION ENGINE ---
int main() {
    // 1. SECURE LIFE-SAFETY RELAY LATCHES IMMEDIATELY
    initEdwardsInterface();
    localBuzzer.init();
    inspectorButton.init();
    statusVisuals.init();

    // 2. ANALYZE AND LOG PERSISTENT HISTORICAL CRASH CODES
    ResetReason bootCondition = detectResetReason();
    logResetToEEPROM(bootCondition);
    printCrashHistory();

    // 3. ARM THE HARDWARE RESET TIMER
    wdt_enable(WDTO_2S); // Enable a 2-second countdown window
    std::cout << "[SYSTEM INITIALIZED] Watchdog Timer armed." << std::endl;

    // 4. CONFIG CONFIGURATION FOR COLD-JUNCTION CRYOGENICS
    if (!initMax31856()) {
        std::cout << "[FATAL BOOT FAULT] MAX31856 sensor array failed to respond!" << std::endl;
        while (true) {
            // Keep loop open to trip Edwards panel and don't reset the watchdog.
            // This forces a hardware reboot sequence while protecting the hallway.
            driveEdwardsInterface(SafetyStatus::SENSOR_FAULT); 
        }
    }

    // Compensate for local ambient freezer compressor heat pollution
    setColdJunctionOffset(-1.5f);

    // Track execution ticks
    uint32_t lastSampleTime = 0;
    uint32_t lastTelemetryTime = 0;

    std::cout << "[RUNTIME ACTIVE] Beginning environment tracking loop..." << std::endl;

    // Local allocations to store live telemetry globally across the iteration threads
    float rawO2 = 0.0f;
    float smoothedO2 = 0.0f;
    float measuredVoltage = 0.0f;
    float rawTemp = 0.0f;
    float smoothedTemp = 0.0f;
    float boardAmbientTemp = 0.0f;
    SafetyStatus compiledStatus = SafetyStatus::SAFE;

    // 5. THE CRITICAL SAFETY ENVIRONMENT LOOP
    while (true) {
        // PET THE HARDWARE WATCHDOG: Continuously pet the watchdog at the top of every cycle.
        // If code freezes anywhere inside this loop, the chip reboots within 2 seconds.
        wdt_reset(); 

        // Replace with your native system clock tracking call (e.g., millis() or get_time())
        uint32_t currentTime = 0; 

        // --- SECTION A: REAL-TIME THREAT SCANNING (Executes exactly every 100ms) ---
        if (currentTime - lastSampleTime >= SAMPLE_INTERVAL_MS) {
            lastSampleTime = currentTime;

            // Gather direct hardware telemetry streams and track electrical loop voltage mappings
            bool o2Healthy = readOxygenLevel(rawO2); 
            bool tempHealthy = readCryoTemperature(rawTemp);

            // Access the underlying voltage read calculation from the oxygen sensor driver logic
            // (Assumes a hook or global tracker is updated by readOxygenLevel)
            measuredVoltage = (rawO2 / O2_PHYSICAL_MAX) * (LOOP_VOLTAGE_MAX - LOOP_VOLTAGE_MIN) + LOOP_VOLTAGE_MIN;

            // Step A1: Formulate physical hardware loop break checks
            bool currentHardwareBroken = (!o2Healthy || !tempHealthy);
            bool hardwareFaultAlarmActive = hardwareFaultDelay.update(currentHardwareBroken, currentTime);

            // Step A2: Formulate environmental crossing states
            bool currentO2Violation = (o2Healthy && (rawO2 <= O2_CRITICAL_THRESHOLD));
            bool currentTempViolation = (tempHealthy && (rawTemp <= TEMP_CRITICAL_THRESHOLD));

            // Run raw inputs through noise filtering and continuous confirmation limits
            smoothedO2 = o2Filter.filter(rawO2);
            smoothedTemp = floorTempFilter.filter(rawTemp);

            bool o2AlarmActive = o2AlarmDelay.update(currentO2Violation, currentTime);
            bool cryoAlarmActive = cryoAlarmDelay.update(currentTempViolation, currentTime);

            // Step A3: Compile active status using strict Life Safety Priority Hierarchy
            compiledStatus = SafetyStatus::SAFE;
            uint8_t visualStatusCode = 0; // 0 = Safe, 1 = Warning, 2 = Critical, 3 = Fault

            if (hardwareFaultAlarmActive) {
                compiledStatus = SafetyStatus::SENSOR_FAULT;
                visualStatusCode = 3;
            } 
            else if (o2AlarmActive || cryoAlarmActive) {
                compiledStatus = SafetyStatus::CRITICAL;
                visualStatusCode = 2;
            } 
            else if (inspectorButton.isPressed(currentTime)) {
                // Manual test functions only run if there is zero active physical danger
                compiledStatus = SafetyStatus::TEST_MODE;
                visualStatusCode = 2; // Test mode mirrors critical flashing red indicators
            }
            else if (o2AlarmDelay.isCountingDown() || hardwareFaultDelay.isCountingDown()) {
                // Pre-alarm grace countdown running
                compiledStatus = SafetyStatus::SAFE;
                visualStatusCode = 1; 
            }

            // Step A4: Route execution outputs to physical hardware layers
            if (compiledStatus == SafetyStatus::TEST_MODE) {
                driveEdwardsInterface(SafetyStatus::CRITICAL); // Open loop for panel verification
            } else {
                driveEdwardsInterface(compiledStatus);        // Process actual real-time status rules
            }
            
            // Drive the LED array using the updated visual code metrics
            statusVisuals.updateDisplay(visualStatusCode, currentTime);

            // Audio management patterns
            if (compiledStatus == SafetyStatus::TEST_MODE) {
                localBuzzer.pulse(currentTime, 100); 
            } 
            else if (compiledStatus == SafetyStatus::CRITICAL || compiledStatus == SafetyStatus::SENSOR_FAULT) {
                // e.g., digitalWrite(PIN_LOCAL_BUZZER, HIGH); // Sound local warning horn constantly
            } 
            else if (visualStatusCode == 1) { // Pre-alarm validation window
                localBuzzer.pulse(currentTime, 150);
            } 
            else {
                localBuzzer.turnOff();
            }
        }

        // --- SECTION B: INTERMITTENT DIAGNOSTICS (Executes every 2000ms) ---
        if (currentTime - lastTelemetryTime >= TELEMETRY_INTERVAL_MS) {
            lastTelemetryTime = currentTime;

            if (readColdJunctionTemperature(boardAmbientTemp)) {
                // Broadcast standard string diagnostics out to the terminal log
                std::cout << "[STATUS] Local Board/Exhaust Temp: " << boardAmbientTemp << " °C" << std::endl;
                
                // CRUCIAL: Stream structural CSV payload frame to synchronize with your interactive Typer tool
                std::cout << "$TELEMETRY," 
                          << rawO2 << "," << smoothedO2 << "," << measuredVoltage << ","
                          << rawTemp << "," << smoothedTemp << "," << boardAmbientTemp << ","
                          << (compiledStatus == SafetyStatus::SAFE ? 1 : 0) 
                          << std::endl;
            }
        }
    }

    return 0; // End of application loop layout
}
