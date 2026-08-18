#pragma once
#include <atomic>
#include <vector>
#include <span>

namespace VoiceClear::Diagnostics {

    /**
     * @brief Lock-free Single Producer Single Consumer (SPSC) queue for telemetry metrics.
     * Ensures the real-time audio thread is never blocked when pushing telemetry data.
     */
    template <typename T>
    class LockFreeTelemetryQueue {
    public:
        explicit LockFreeTelemetryQueue(size_t capacity) 
            : m_capacity(capacity), m_buffer(capacity) {}

        bool Push(const T& item) {
            auto currentTail = m_tail.load(std::memory_order_relaxed);
            auto nextTail = (currentTail + 1) % m_capacity;
            if (nextTail == m_head.load(std::memory_order_acquire)) {
                return false; // Queue full
            }
            m_buffer[currentTail] = item;
            m_tail.store(nextTail, std::memory_order_release);
            return true;
        }

        bool Pop(T& item) {
            auto currentHead = m_head.load(std::memory_order_relaxed);
            if (currentHead == m_tail.load(std::memory_order_acquire)) {
                return false; // Queue empty
            }
            item = m_buffer[currentHead];
            m_head.store((currentHead + 1) % m_capacity, std::memory_order_release);
            return true;
        }

    private:
        size_t m_capacity;
        std::vector<T> m_buffer;
        alignas(64) std::atomic<size_t> m_head{0};
        alignas(64) std::atomic<size_t> m_tail{0};
    };
}
