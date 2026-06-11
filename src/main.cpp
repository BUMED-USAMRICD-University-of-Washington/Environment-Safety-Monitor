include <iostream>
include <cstdint>

include "max31856.h"
include "oxygen_sensor.h"
include "moving_average.h"
include "alarm_latch.h"
include "buzzer_driver.h"
include "test_button.h"
include "crash_logger.h"
include "led_driver.h"
include "telemetry_types.h"
include "spsc_ring_buffer.h"

constexpr float O2_CRITICAL_THRESHOLD    = 19.5f;   
constexpr float TEMP_CRITICAL_THRESHOLD  = 0.0f;    
constexpr uint32_t SAMPLE_INTERVAL_MS    = 100;     
constexpr uint8_t PIN_EDWARDS_RELAY      = 3;       

MovingAverage<float, 10> floorTempFilter;
MovingAverage<float, 15> o2Filter;

AlarmLatch o2AlarmDelay(3000);        
AlarmLatch cryoAlarmDelay(0);         
AlarmLatch hardwareFaultDelay(1000);  

BuzzerDriver localBuzzer;
TestButton inspectorButton(PIN_TEST_BUTTON, TEST_DEBOUNCE_MS);
LedDriver statusVisuals;

LockFreeQueue<AlarmTelemetryPacket, 64> telemetry_queue;

void initEdwardsInterface() {
}

void driveEdwardsInterface(uint8_t statusMask) {
    if (statusMask == SafetySystem::STATE_NORMAL) return;
    
    if (statusMask & SafetySystem::CRIT_O2_HYPOXIA) return; 
    if (statusMask & SafetySystem::CRIT_CRYO_FAILURE) return; 
    if (statusMask & SafetySystem::FAULT_O2_WIRE_BREAK) return; 
}

void transmitEdwardsWarning(uint8_t statusMask) {
    if ((statusMask & SafetySystem::CRIT_O2_HYPOXIA) == 0) return;

    std::cout << "\n[EDWARDS RELAY TRIPPED] PRIORITY 1 EVACUATION ALARM\n";
    std::cout << "[MEDICAL WARNING] Hypoxia-Induced Anosognosia Detected.\n";
    std::cout << "Victims will not realize they are suffocating. Expect severe intoxication, paranoia, and combativeness.\n";
    std::cout << "Force evacuation immediately. Do not attempt rescue without independent air supply.\n";
}

uint8_t evaluateVisualStatus(uint8_t compiledStatus, bool countdown) {
    if (compiledStatus & SafetySystem::FAULT_O2_WIRE_BREAK) return 3;
    if (compiledStatus & SafetySystem::FAULT_TEMP_SPI_DISCON) return 3;
    if (compiledStatus & SafetySystem::CRIT_O2_HYPOXIA) return 2;
    if (compiledStatus & SafetySystem::CRIT_CRYO_FAILURE) return 2;
    if (countdown) return 1;
    return 0;
}

void updateAudioAlerts(uint8_t compiledStatus, uint8_t visualCode, uint32_t currentTime) {
    if (compiledStatus & SafetySystem::CRIT_O2_HYPOXIA) return; 
    if (compiledStatus & SafetySystem::CRIT_CRYO_FAILURE) return; 

    if (visualCode == 1) {
        localBuzzer.pulse(currentTime, 150);
        return;
    }
    if (visualCode == 3) {
        localBuzzer.pulse(currentTime, 500); 
        return;
    }
    
    localBuzzer.turnOff();
}

void Core0_SafetyLoop(void * pvParameters) {
    initEdwardsInterface();
    localBuzzer.init();
    inspectorButton.init();
    statusVisuals.init();

    if (!initMax31856()) {
        std::cout << "[FATAL BOOT FAULT] MAX31856 cryogenic array offline!\n";
        while (true) {
            driveEdwardsInterface(SafetySystem::FAULT_TEMP_SPI_DISCON);
        }
    }

    setColdJunctionOffset(-1.5f);

    uint32_t lastSampleTime = 0;
    uint16_t loop_tracker = 0;

    while (true) {
        uint32_t currentTime = 0; 

        if (currentTime - lastSampleTime < SAMPLE_INTERVAL_MS) continue; 
        lastSampleTime = currentTime;

        AlarmTelemetryPacket current_packet;
        current_packet.timestamp_ms = currentTime;
        current_packet.loop_counter = loop_tracker++;
        current_packet.alarm_state = SafetySystem::STATE_NORMAL;

        float rawO2 = 0.0f;
        float rawTemp = 0.0f;

        bool o2Healthy = readOxygenLevel(rawO2);
        bool tempHealthy = readCryoTemperature(rawTemp);

        current_packet.oxygen_level = o2Filter.filter(rawO2);
        current_packet.floor_temp = floorTempFilter.filter(rawTemp);

        bool currentHardwareBroken = (!o2Healthy || !tempHealthy);
        bool currentO2Violation = (o2Healthy && (current_packet.oxygen_level <= O2_CRITICAL_THRESHOLD));
        bool currentTempViolation = (tempHealthy && (current_packet.floor_temp <= TEMP_CRITICAL_THRESHOLD));

        bool hardwareFaultAlarmActive = hardwareFaultDelay.update(currentHardwareBroken, currentTime);
        bool o2AlarmActive = o2AlarmDelay.update(currentO2Violation, currentTime);
        bool cryoAlarmActive = cryoAlarmDelay.update(currentTempViolation, currentTime);

        if (hardwareFaultAlarmActive) current_packet.alarm_state |= SafetySystem::FAULT_O2_WIRE_BREAK;
        if (o2AlarmActive) current_packet.alarm_state |= SafetySystem::CRIT_O2_HYPOXIA;
        if (cryoAlarmActive) current_packet.alarm_state |= SafetySystem::CRIT_CRYO_FAILURE;

        if (inspectorButton.isPressed(currentTime)) current_packet.alarm_state |= SafetySystem::CRIT_O2_HYPOXIA;

        bool countdownActive = o2AlarmDelay.isCountingDown() || hardwareFaultDelay.isCountingDown();
        uint8_t visualStatusCode = evaluateVisualStatus(current_packet.alarm_state, countdownActive);
        
        statusVisuals.updateDisplay(visualStatusCode, currentTime);
        updateAudioAlerts(current_packet.alarm_state, visualStatusCode, currentTime);

        transmitEdwardsWarning(current_packet.alarm_state);
        driveEdwardsInterface(current_packet.alarm_state);

        telemetry_queue.push(current_packet);
    }
}

void Core1_TelemetryLoop(void * pvParameters) {
    ResetReason bootCondition = detectResetReason();
    logResetToEEPROM(bootCondition);
    printCrashHistory();

    AlarmTelemetryPacket received_packet;

    while(true) {
        if (!telemetry_queue.pop(received_packet)) continue; 

        std::cout << "[$TELEMETRY] O2: " << received_packet.oxygen_level 
                  << "% | Temp: " << received_packet.floor_temp 
                  << "C | Code: " << static_cast<int>(received_packet.alarm_state) 
                  << "\n";
                  
        if (received_packet.alarm_state > 0) {
        }
    }
}

int main() {
    xTaskCreatePinnedToCore(Core0_SafetyLoop, "SafetyLoop", 4096, NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore(Core1_TelemetryLoop, "USBTelemetry", 4096, NULL, 1, NULL, 1);
    
    return 0;
}
