#pragma once
#include <atomic>
#include <chrono>
#include <mutex>
#include <vector>

namespace VoiceClear::Profiling {

    /**
     * @brief Specialized lock-free latency profiler for AI inference metrics.
     */
    class AIProfiler {
    public:
        AIProfiler();
        ~AIProfiler() = default;

        /**
         * @brief Logs the duration of a single inference run.
         */
        void RecordInference(float durationMs) noexcept;

        /**
         * @brief Increments the number of times recurrent states were reset.
         */
        void IncrementResetCount() noexcept;

        // Metric extraction (Thread-safe, can be polled by Diagnostics UI)
        float GetAverageInferenceMs() const noexcept;
        float GetPeakInferenceMs() const noexcept;
        float GetP95InferenceMs() const noexcept;
        float GetP99InferenceMs() const noexcept;
        
        uint64_t GetInferenceCount() const noexcept;
        uint64_t GetResetCount() const noexcept;
        float GetModelUptimeSeconds() const noexcept;

    private:
        std::chrono::time_point<std::chrono::steady_clock> m_startTime;
        
        std::atomic<uint64_t> m_inferenceCount{0};
        std::atomic<uint64_t> m_resetCount{0};
        
        // Ring buffer for P95/P99 calculation without locks
        static constexpr size_t HISTORY_SIZE = 1000;
        float m_history[HISTORY_SIZE];
        std::atomic<size_t> m_historyIndex{0};

        std::atomic<float> m_peakMs{0.0f};
        std::atomic<float> m_totalMs{0.0f};
    };

}
