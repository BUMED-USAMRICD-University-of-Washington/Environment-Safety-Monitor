#include <cstdint>

// --- SYSTEM CONFIGURATION ---
constexpr float O2_CRITICAL_THRESHOLD  = 19.5f;  // OSHA safe limit (%)
constexpr float TEMP_CRITICAL_THRESHOLD = 0.0f;   // Cryogenic drop limit (°C)
constexpr uint32_t SAMPLE_INTERVAL_MS  = 100;    // Non-blocking loop rate

// --- HARDWARE PIN MAPPING (Example Configuration) ---
constexpr uint8_t PIN_ALARM_STROBE     = 2;
constexpr uint8_t PIN_SAFETY_RELAY     = 3;      // Connected to BMS (Active LOW / Fail-Safe)
constexpr uint8_t PIN_O2_SENSOR_ADC    = A0;

// --- SYSTEM STATE ---
enum class SafetyStatus : uint8_t {
    SAFE,
    WARNING,
    CRITICAL,
    SENSOR_FAULT
};

// --- HARDWARE ABSTRACTION LAYER STUBS ---
// Replace these with your actual hardware driver calls (e.g., SPI/I2C/ADC)
void initHardware() {
    // Configure PIN_ALARM_STROBE as OUTPUT
    // Configure PIN_SAFETY_RELAY as OUTPUT (Set HIGH immediately to secure fail-safe loop)
}

float readOxygenLevel() {
    // Read your 4-20mA sensor converted to 0-3.3V/0-5V analog voltage
    // Convert voltage linearly to 0.0% - 25.0% O2
    return 20.9f; 
}

float readCryoTemperature() {
    // Interface with your MAX31856 SPI Type-T Thermocouple amplifier here
    return 22.5f;
}

void triggerAlarms(SafetyStatus status) {
    if (status == SafetyStatus::CRITICAL || status == SafetyStatus::SENSOR_FAULT) {
        // OPEN THE RELAY (Set LOW) to signal the Facility Building Management System
        // Turn on the strobe light and horn
    } else {
        // KEEP THE RELAY CLOSED (Set HIGH) - Everything is safe
        // Turn off strobe light and horn
    }
}

// --- MAIN MONITORING LOGIC ---
SafetyStatus evaluateSafety(float o2, float temp) {
    // Out-of-bounds voltage checks to trap disconnected or broken sensors
    if (o2 < 0.0f || o2 > 30.0f || temp < -250.0f) {
        return SafetyStatus::SENSOR_FAULT;
    }
    
    // Critical air displacement or liquid leak detected
    if (o2 <= O2_CRITICAL_THRESHOLD || temp <= TEMP_CRITICAL_THRESHOLD) {
        return SafetyStatus::CRITICAL;
    }
    
    return SafetyStatus::SAFE;
}

int main() {
    initHardware();
    
    uint32_t lastSampleTime = 0; // Track system time
    
    // Infinite safety-critical execution loop
    while (true) {
        uint32_t currentTime = 0; // Replace with your hardware timer: e.g., millis() or get_time()
        
        // Non-blocking execution to ensure deterministic sample intervals
        if (currentTime - lastSampleTime >= SAMPLE_INTERVAL_MS) {
            lastSampleTime = currentTime;
            
            // 1. Gather sensor telemetry
            float currentO2 = readOxygenLevel();
            float currentTemp = readCryoTemperature();
            
            // 2. Evaluate system health against criteria
            SafetyStatus currentStatus = evaluateSafety(currentO2, currentTemp);
            
            // 3. Drive hardware responses instantly
            triggerAlarms(currentStatus);
        }
    }
    return 0;
}
