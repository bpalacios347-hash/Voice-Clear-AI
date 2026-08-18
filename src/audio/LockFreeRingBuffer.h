#pragma once
#include <atomic>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <span>

namespace VoiceClear::Audio {

    /**
     * @brief High-performance Single Producer Single Consumer (SPSC) Lock-Free Ring Buffer.
     * Guaranteed zero allocations after initialization, no mutexes, and cache-line aligned
     * to prevent false sharing between the capture (producer) and processing (consumer) threads.
     */
    template <typename T>
    class LockFreeRingBuffer {
    public:
        explicit LockFreeRingBuffer(size_t capacity) 
            : m_capacity(capacity + 1), m_buffer(capacity + 1) {
            // capacity + 1 is used to distinguish between full and empty states
            Reset();
        }

        ~LockFreeRingBuffer() = default;

        // Disallow copying and assignment to ensure strict SPSC semantics
        LockFreeRingBuffer(const LockFreeRingBuffer&) = delete;
        LockFreeRingBuffer& operator=(const LockFreeRingBuffer&) = delete;

        /**
         * @brief Pushes multiple items into the buffer wait-free.
         * @return The number of items successfully pushed.
         */
        size_t Push(const T* data, size_t count) noexcept {
            const size_t head = m_head.load(std::memory_order_relaxed);
            const size_t tail = m_tail.load(std::memory_order_acquire);
            
            size_t available = (tail > head) ? (tail - head - 1) : (m_capacity - head + tail - 1);
            size_t toWrite = std::min(count, available);

            if (toWrite == 0) return 0;

            size_t firstChunk = std::min(toWrite, m_capacity - head);
            std::copy_n(data, firstChunk, m_buffer.data() + head);
            
            if (firstChunk < toWrite) {
                std::copy_n(data + firstChunk, toWrite - firstChunk, m_buffer.data());
            }

            m_head.store((head + toWrite) % m_capacity, std::memory_order_release);
            return toWrite;
        }

        /**
         * @brief Pops multiple items from the buffer wait-free.
         * @return The number of items successfully popped.
         */
        size_t Pop(T* data, size_t count) noexcept {
            const size_t tail = m_tail.load(std::memory_order_relaxed);
            const size_t head = m_head.load(std::memory_order_acquire);
            
            size_t available = (head >= tail) ? (head - tail) : (m_capacity - tail + head);
            size_t toRead = std::min(count, available);

            if (toRead == 0) return 0;

            size_t firstChunk = std::min(toRead, m_capacity - tail);
            std::copy_n(m_buffer.data() + tail, firstChunk, data);

            if (firstChunk < toRead) {
                std::copy_n(m_buffer.data(), toRead - firstChunk, data + firstChunk);
            }

            m_tail.store((tail + toRead) % m_capacity, std::memory_order_release);
            return toRead;
        }

        void Clear() noexcept { Reset(); }
        void Reset() noexcept {
            m_head.store(0, std::memory_order_release);
            m_tail.store(0, std::memory_order_release);
        }

        size_t Capacity() const noexcept { return m_capacity - 1; }
        
        size_t Size() const noexcept { return Occupancy(); }
        
        size_t Occupancy() const noexcept {
            const size_t head = m_head.load(std::memory_order_acquire);
            const size_t tail = m_tail.load(std::memory_order_acquire);
            return (head >= tail) ? (head - tail) : (m_capacity - tail + head);
        }

        /// Alias for Occupancy() — number of items available to read.
        size_t ReadAvailable() const noexcept { return Occupancy(); }

        /// Number of items that can be written before the buffer is full.
        size_t WriteAvailable() const noexcept {
            return m_capacity - 1 - Occupancy();
        }

        bool Empty() const noexcept {
            return m_head.load(std::memory_order_acquire) == m_tail.load(std::memory_order_acquire);
        }

        bool Full() const noexcept {
            const size_t head = m_head.load(std::memory_order_acquire);
            const size_t tail = m_tail.load(std::memory_order_acquire);
            return ((head + 1) % m_capacity) == tail;
        }

    private:
        size_t m_capacity;
        std::vector<T> m_buffer;

        // Force cache-line alignment (typically 64 bytes) to prevent false sharing 
        // between the Producer core and Consumer core.
        alignas(64) std::atomic<size_t> m_head{0}; // Written by Producer (Push)
        alignas(64) std::atomic<size_t> m_tail{0}; // Written by Consumer (Pop)
    };

}
