#ifndef ALARM_LATCH_H
#define ALARM_LATCH_H

#include <cstdint>

class AlarmLatch {
public:
    /**
     * @param[in] delayDurationMs The continuous time required in violation to trip (e.g., 3000ms).
     */
    explicit AlarmLatch(uint32_t delayDurationMs) 
        : m_delayDurationMs(delayDurationMs), m_violationStartTime(0), m_isViolated(false), m_isLatched(false) {}

    /**
     * Evaluates a sensor condition against an active threat state.
     * @param[in] conditionActive True if the raw sensor is currently violating a safe threshold.
     * @param[in] currentTimeMs The current running time ticks from your hardware clock.
     * @return bool True if the hazard has broken past the time delay buffer and is locked into an active alarm.
     */
    bool update(bool conditionActive, uint32_t currentTimeMs) {
        if (!conditionActive) {
            // TRANSIENT OVER / HAZARD CLEARED: Instantly reset the verification timer.
            // This prevents brief, intermittent spikes from slowly building up to a fault.
            m_isViolated = false;
            m_violationStartTime = 0;
            m_isLatched = false;
            return false;
        }

        // Hazard condition is active. Check if this is the very first moment it crossed the line:
        if (!m_isViolated) {
            m_isViolated = true;
            m_violationStartTime = currentTimeMs; // Lock in the exact moment the threat began
            return false;                          // Do not sound the alarm yet; wait for verification
        }

        // If the threat has persisted continuously longer than our allowed grace period:
        if (currentTimeMs - m_violationStartTime >= m_delayDurationMs) {
            m_isLatched = true;
        }

        return m_isLatched;
    }

private:
    uint32_t m_delayDurationMs;   // Target delay window in milliseconds
    uint32_t m_violationStartTime; // Timestamp when the threat first crossed the line
    bool m_isViolated;             // Tracks if a validation countdown is currently underway
    bool m_isLatched;              // Tracks if the alarm state has broken through the time gate
};

#endif // ALARM_LATCH_H
