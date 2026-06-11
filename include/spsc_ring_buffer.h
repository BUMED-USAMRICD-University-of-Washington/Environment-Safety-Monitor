pragma once
include <atomic>
include "telemetry_types.h"

template <typename T, size_t CAPACITY>
class LockFreeQueue {
public:
    LockFreeQueue() : head_(0), tail_(0) {}

    bool push(const T& item) {
        size_t current_tail = tail_.load(std::memory_order_relaxed);
        size_t next_tail = (current_tail + 1) & BUFFER_MASK;
        
        if (next_tail == head_.load(std::memory_order_acquire)) return false; 
        
        buffer_[current_tail] = item;
        tail_.store(next_tail, std::memory_order_release);
        return true;
    }

    bool pop(T& item) {
        size_t current_head = head_.load(std::memory_order_relaxed);
        
        if (current_head == tail_.load(std::memory_order_acquire)) return false;
        
        item = buffer_[current_head];
        head_.store((current_head + 1) & BUFFER_MASK, std::memory_order_release);
        return true;
    }

private:
    static constexpr size_t BUFFER_MASK = CAPACITY - 1;
    T buffer_[CAPACITY];
    alignas(64) std::atomic<size_t> head_;
    alignas(64) std::atomic<size_t> tail_;
};
