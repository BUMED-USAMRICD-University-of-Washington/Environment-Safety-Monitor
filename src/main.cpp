include <iostream>
include <cstdint>
include <avr/wdt.h>  

include "max31856.h"
include "oxygen_sensor.h"
include "moving_average.h"
include "alarm_latch.h"
include "buzzer_driver.h"
include "test_button.h"
include "crash_logger.h"
include "led_driver.h"

constexpr float O2_CRITICAL_THRESHOLD    = 19.5f;   
constexpr float TEMP_CRITICAL_THRESHOLD  = 0.0f;    
constexpr uint32_t SAMPLE_INTERVAL_MS    = 100;     
constexpr uint32_t TELEMETRY_INTERVAL_MS = 2000;    
constexpr uint8_t PIN_EDWARDS_RELAY      = 3;       

enum class SafetyStatus : uint8_t { SAFE, WARNING, CRITICAL, SENSOR_FAULT, TEST_MODE };

MovingAverage<float, 10> floorTempFilter;
MovingAverage<float, 15> o2Filter;

AlarmLatch o2AlarmDelay(3000);        
AlarmLatch cryoAlarmDelay(0);         
AlarmLatch hardwareFaultDelay(1000);  

BuzzerDriver localBuzzer;
TestButton inspectorButton(PIN_TEST_BUTTON, TEST_DEBOUNCE_MS);
LedDriver statusVisuals;

void initEdwardsInterface() {
}

void driveEdwardsInterface(SafetyStatus status) {
    if (status == SafetyStatus::SAFE) return;
    if (status == SafetyStatus::CRITICAL) return;
    if (status == SafetyStatus::SENSOR_FAULT) return;
}

SafetyStatus evaluateCompiledStatus(bool hardwareFault, bool o2Alarm, bool cryoAlarm, bool testPressed) {
    if (hardwareFault) return SafetyStatus::SENSOR_FAULT;
    if (o2Alarm || cryoAlarm) return SafetyStatus::CRITICAL;
    if (testPressed) return SafetyStatus::TEST_MODE;
    return SafetyStatus::SAFE;
}

uint8_t evaluateVisualStatus(SafetyStatus status, bool countdown) {
    if (status == SafetyStatus::SENSOR_FAULT) return 3;
    if (status == SafetyStatus::CRITICAL || status == SafetyStatus::TEST_MODE) return 2;
    if (countdown) return 1;
    return 0;
}

void updateAudioAlerts(SafetyStatus status, uint8_t visualCode, uint32_t currentTime) {
    if (status == SafetyStatus::TEST_MODE) {
        localBuzzer.pulse(currentTime, 100); 
        return;
    }
    if (status == SafetyStatus::CRITICAL || status == SafetyStatus::SENSOR_FAULT) {
        return;
    }
    if (visualCode == 1) {
        localBuzzer.pulse(currentTime, 150);
        return;
    }
    localBuzzer.turnOff();
}

int main() {
    initEdwardsInterface();
    localBuzzer.init();
    inspectorButton.init();
    statusVisuals.init();

    ResetReason bootCondition = detectResetReason();
    logResetToEEPROM(bootCondition);
    printCrashHistory();

    wdt_enable(WDTO_2S); 
    std::cout << "[SYSTEM INITIALIZED] Watchdog Timer armed." << std::endl;

    if (!initMax31856()) {
        std::cout << "[FATAL BOOT FAULT] MAX31856 array offline!" << std::endl;
        while (true) {
            driveEdwardsInterface(SafetyStatus::SENSOR_FAULT); 
        }
    }

    setColdJunctionOffset(-1.5f);

    uint32_t lastSampleTime = 0;
    uint32_t lastTelemetryTime = 0;

    float rawO2 = 0.0f;
    float smoothedO2 = 0.0f;
    float measuredVoltage = 0.0f;
    float rawTemp = 0.0f;
    float smoothedTemp = 0.0f;
    float boardAmbientTemp = 0.0f;
    SafetyStatus compiledStatus = SafetyStatus::SAFE;

    while (true) {
        wdt_reset(); 
        uint32_t currentTime = 0; 

        if (currentTime - lastSampleTime >= SAMPLE_INTERVAL_MS) {
            lastSampleTime = currentTime;

            bool o2Healthy = readOxygenLevel(rawO2); 
            bool tempHealthy = readCryoTemperature(rawTemp);

            measuredVoltage = (rawO2 / O2_PHYSICAL_MAX) * (LOOP_VOLTAGE_MAX - LOOP_VOLTAGE_MIN) + LOOP_VOLTAGE_MIN;

            bool currentHardwareBroken = (!o2Healthy || !tempHealthy);
            bool hardwareFaultAlarmActive = hardwareFaultDelay.update(currentHardwareBroken, currentTime);

            bool currentO2Violation = (o2Healthy && (rawO2 <= O2_CRITICAL_THRESHOLD));
            bool currentTempViolation = (tempHealthy && (rawTemp <= TEMP_CRITICAL_THRESHOLD));

            smoothedO2 = o2Filter.filter(rawO2);
            smoothedTemp = floorTempFilter.filter(rawTemp);

            bool o2AlarmActive = o2AlarmDelay.update(currentO2Violation, currentTime);
            bool cryoAlarmActive = cryoAlarmDelay.update(currentTempViolation, currentTime);
            bool countdownActive = o2AlarmDelay.isCountingDown() || hardwareFaultDelay.isCountingDown();

            compiledStatus = evaluateCompiledStatus(
                hardwareFaultAlarmActive, 
                o2AlarmActive, 
                cryoAlarmActive, 
                inspectorButton.isPressed(currentTime)
            );
            
            uint8_t visualStatusCode = evaluateVisualStatus(compiledStatus, countdownActive);

            if (compiledStatus == SafetyStatus::TEST_MODE) driveEdwardsInterface(SafetyStatus::CRITICAL); 
            if (compiledStatus != SafetyStatus::TEST_MODE) driveEdwardsInterface(compiledStatus);        
            
            statusVisuals.updateDisplay(visualStatusCode, currentTime);
            updateAudioAlerts(compiledStatus, visualStatusCode, currentTime);
        }

        if (currentTime - lastTelemetryTime >= TELEMETRY_INTERVAL_MS) {
            lastTelemetryTime = currentTime;
            if (readColdJunctionTemperature(boardAmbientTemp)) {
                std::cout << "[STATUS] Local Board/Exhaust Temp: " << boardAmbientTemp << " °C" << std::endl;
            }
        }
    }
    return 0; 
}
