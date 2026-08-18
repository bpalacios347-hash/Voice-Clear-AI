#pragma once

namespace VoiceClear::Diagnostics {
    
    struct TelemetryData {
        float currentLatencyMs;
        float peakLatencyMs;
        float currentCpuUsage;
        float peakCpuUsage;
        float inferenceTimeMs;
        float inputRmsLevel;
        float outputRmsLevel;
        int activeSampleRate;
        int xrunCount;
        int droppedFrames;
    };

    /**
     * @brief Manages lock-free telemetry streaming from the Audio Engine to the IPC layer.
     */
    class TelemetryManager {
    public:
        TelemetryManager() = default;
        ~TelemetryManager() = default;

        void PublishInferenceTime(float ms);
        void PublishLatency(float ms);
        void PublishCpuUsage(float percent);
        void PublishInputLevel(float rms);
        void PublishOutputLevel(float rms);
        void IncrementXrun();

        // Fetches aggregated snapshot for the IPC thread (non-blocking)
        TelemetryData GetSnapshot() const;

        /// Resets all counters. Called by the Watchdog after graceful recovery.
        void Reset() noexcept;
    };
}
