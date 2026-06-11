ifndef MOVING_AVERAGE_H
define MOVING_AVERAGE_H

include <cstddef>

template <typename T, size_t WindowSize>
class MovingAverage {
public:
    MovingAverage() : m_history{}, m_index(0), m_count(0), m_sum(0.0f) {}

    T filter(T newValue) {
        if (m_count == WindowSize) {
            m_sum -= m_history[m_index];
        } 
        
        if (m_count < WindowSize) {
            m_count++;
        }

        m_history[m_index] = newValue;
        m_sum += newValue;

        m_index = (m_index + 1) % WindowSize;

        return m_sum / static_cast<T>(m_count);
    }

private:
    T m_history[WindowSize]; 
    size_t m_index;         
    size_t m_count;         
    T m_sum;                
};

endif
