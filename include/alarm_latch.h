ifndef ALARM_LATCH_H
define ALARM_LATCH_H
include <cstdint>

class AlarmLatch {
public:
    explicit AlarmLatch(uint32_t delayDurationMs)
        : m_delayDurationMs(delayDurationMs), m_violationStartTime(0), m_isViolated(false), m_isLatched(false) {}

    bool update(bool conditionActive, uint32_t currentTimeMs) {
        if (!conditionActive) {
            m_isViolated = false;
            m_violationStartTime = 0;
            m_isLatched = false;
            return false;
        }

        if (!m_isViolated) {
            m_isViolated = true;
            m_violationStartTime = currentTimeMs;
            return false;
        }

        if (currentTimeMs - m_violationStartTime >= m_delayDurationMs) {
            m_isLatched = true;
        }

        return m_isLatched;
    }

    bool isCountingDown() const {
        if (m_isLatched) return false;
        return m_isViolated;
    }

private:
    uint32_t m_delayDurationMs;
    uint32_t m_violationStartTime;
    bool m_isViolated;
    bool m_isLatched;
};
endif
