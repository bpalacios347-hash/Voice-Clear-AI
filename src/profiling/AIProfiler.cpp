#include "AIProfiler.h"
#include <algorithm>
#include <cmath>

namespace VoiceClear::Profiling {

    AIProfiler::AIProfiler() {
        m_startTime = std::chrono::steady_clock::now();
        for (size_t i = 0; i < HISTORY_SIZE; ++i) {
            m_history[i] = 0.0f;
        }
    }

    void AIProfiler::RecordInference(float durationMs) noexcept {
        m_inferenceCount.fetch_add(1, std::memory_order_relaxed);
        
        // Update peak using CAS loop for lock-free max
        float currentPeak = m_peakMs.load(std::memory_order_relaxed);
        while (durationMs > currentPeak && !m_peakMs.compare_exchange_weak(currentPeak, durationMs, std::memory_order_relaxed)) {
            // Loop until updated or currentPeak is higher
        }

        // Running total for average
        float currentTotal = m_totalMs.load(std::memory_order_relaxed);
        while (!m_totalMs.compare_exchange_weak(currentTotal, currentTotal + durationMs, std::memory_order_relaxed)) {}

        // Circular history for percentiles
        size_t idx = m_historyIndex.fetch_add(1, std::memory_order_relaxed) % HISTORY_SIZE;
        m_history[idx] = durationMs;
    }

    void AIProfiler::IncrementResetCount() noexcept {
        m_resetCount.fetch_add(1, std::memory_order_relaxed);
    }

    float AIProfiler::GetAverageInferenceMs() const noexcept {
        uint64_t count = m_inferenceCount.load(std::memory_order_relaxed);
        if (count == 0) return 0.0f;
        return m_totalMs.load(std::memory_order_relaxed) / static_cast<float>(count);
    }

    float AIProfiler::GetPeakInferenceMs() const noexcept {
        return m_peakMs.load(std::memory_order_relaxed);
    }

    float AIProfiler::GetP95InferenceMs() const noexcept {
        uint64_t count = std::min(m_inferenceCount.load(std::memory_order_relaxed), static_cast<uint64_t>(HISTORY_SIZE));
        if (count == 0) return 0.0f;

        std::vector<float> sorted(m_history, m_history + count);
        std::sort(sorted.begin(), sorted.end());
        
        size_t idx = static_cast<size_t>(std::ceil(count * 0.95)) - 1;
        return sorted[std::min(idx, count - 1)];
    }

    float AIProfiler::GetP99InferenceMs() const noexcept {
        uint64_t count = std::min(m_inferenceCount.load(std::memory_order_relaxed), static_cast<uint64_t>(HISTORY_SIZE));
        if (count == 0) return 0.0f;

        std::vector<float> sorted(m_history, m_history + count);
        std::sort(sorted.begin(), sorted.end());
        
        size_t idx = static_cast<size_t>(std::ceil(count * 0.99)) - 1;
        return sorted[std::min(idx, count - 1)];
    }

    uint64_t AIProfiler::GetInferenceCount() const noexcept {
        return m_inferenceCount.load(std::memory_order_relaxed);
    }

    uint64_t AIProfiler::GetResetCount() const noexcept {
        return m_resetCount.load(std::memory_order_relaxed);
    }

    float AIProfiler::GetModelUptimeSeconds() const noexcept {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration<float>(now - m_startTime).count();
    }
}
