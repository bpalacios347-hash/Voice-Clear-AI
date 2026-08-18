#pragma once
#include <cstdint>
#include <memory>
#include <atomic>
#include "interfaces/IAudioStage.h"
#include "../diagnostics/TelemetryManager.h"

namespace VoiceClear::Audio {
    
    struct ClockStatistics {
        double averageLatencyMs{0.0};
        double peakLatencyMs{0.0};
        double jitterVarianceMs{0.0};
        double maxJitterMs{0.0};
        double driftMs{0.0};
    };

    /**
     * @brief High-precision authoritative clock for the audio engine.
     * Calculates end-to-end latency, jitter, and clock drift lock-free.
     */
    class AudioClock {
    public:
        explicit AudioClock(std::shared_ptr<Diagnostics::TelemetryManager> telemetry);
        ~AudioClock() = default;

        // High resolution QPC wrapper
        static uint64_t Now() noexcept;
        static double TicksToMs(uint64_t ticks) noexcept;

        void Reset() noexcept;
        
        // Timing stamps (mutate the block directly, no locking)
        void RecordCaptureTimestamp(AudioBuffer& buffer) const noexcept;
        void RecordRingBufferEntry(AudioBuffer& buffer) const noexcept;
        void RecordRingBufferExit(AudioBuffer& buffer) const noexcept;
        void RecordRenderTimestamp(AudioBuffer& buffer) const noexcept;

        // Analysis (called on the consumer thread just before rendering)
        void AnalyzeBlock(const AudioBuffer& buffer) noexcept;
        
        ClockStatistics GetStatistics() const noexcept;

    private:
        std::shared_ptr<Diagnostics::TelemetryManager> m_telemetry;

        // Lock-free metric storage
        std::atomic<uint64_t> m_totalBlocks{0};
        std::atomic<double> m_avgLatencyMs{0.0};
        std::atomic<double> m_peakLatencyMs{0.0};
        
        std::atomic<double> m_maxJitterMs{0.0};
        std::atomic<double> m_jitterSum{0.0}; // Simplified for variance calculation
        std::atomic<uint64_t> m_lastRenderTick{0};

        std::atomic<uint64_t> m_firstCaptureTick{0};
        std::atomic<uint64_t> m_firstRenderTick{0};
        std::atomic<double> m_driftMs{0.0};
    };

}
