#ifndef MOVING_AVERAGE_H
#define MOVING_AVERAGE_H

#include <cstddef>

/**
 * A memory-safe, zero-allocation Moving Average Filter using a circular buffer.
 * @tparam T The data type to filter (e.g., float or double).
 * @tparam WindowSize The number of data points to average over.
 */
template <typename T, size_t WindowSize>
class MovingAverage {
public:
    MovingAverage() : m_history{}, m_index(0), m_count(0), m_sum(0.0f) {}

    /**
     * Pushes a new raw reading into the filter and calculates the updated average.
     * @param newValue The raw, noisy sensor value.
     * @return The smoothed, filtered value.
     */
    T filter(T newValue) {
        // If the buffer is full, subtract the oldest value from the running sum
        if (m_count == WindowSize) {
            m_sum -= m_history[m_index];
        } else {
            m_count++;
        }

        // Store the new value and add it to the running sum
        m_history[m_index] = newValue;
        m_sum += newValue;

        // Advance the index circularly
        m_index = (m_index + 1) % WindowSize;

        // Return the mathematically smoothed average
        return m_sum / static_cast<T>(m_count);
    }

private:
    T m_history[WindowSize]; // Fixed-size array allocated on stack/data segment
    size_t m_index;         // Tracks the oldest data location
    size_t m_count;         // Tracks how many samples have been gathered so far
    T m_sum;                // Running total to ensure O(1) lightning-fast execution
};

#endif // MOVING_AVERAGE_H
