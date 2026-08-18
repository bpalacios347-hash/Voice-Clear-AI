#include "TelemetryManager.h"
#include <atomic>
#include <algorithm>

namespace VoiceClear::Diagnostics {

    // Internal state using atomics for lock-free updates from audio thread
    struct TelemetryState {
        std::atomic<float> currentLatencyMs{0.0f};
        std::atomic<float> peakLatencyMs{0.0f};
        std::atomic<float> currentCpuUsage{0.0f};
        std::atomic<float> peakCpuUsage{0.0f};
        std::atomic<float> inferenceTimeMs{0.0f};
        std::atomic<float> inputRmsLevel{0.0f};
        std::atomic<float> outputRmsLevel{0.0f};
        std::atomic<int> activeSampleRate{48000};
        std::atomic<int> xrunCount{0};
        std::atomic<int> droppedFrames{0};
    };

    static TelemetryState g_state;

    void TelemetryManager::PublishInferenceTime(float ms) {
        g_state.inferenceTimeMs.store(ms, std::memory_order_relaxed);
    }

    void TelemetryManager::PublishLatency(float ms) {
        g_state.currentLatencyMs.store(ms, std::memory_order_relaxed);
        
        float currentPeak = g_state.peakLatencyMs.load(std::memory_order_relaxed);
        if (ms > currentPeak) {
            g_state.peakLatencyMs.store(ms, std::memory_order_relaxed);
        }
    }

    void TelemetryManager::PublishCpuUsage(float percent) {
        g_state.currentCpuUsage.store(percent, std::memory_order_relaxed);
        
        float currentPeak = g_state.peakCpuUsage.load(std::memory_order_relaxed);
        if (percent > currentPeak) {
            g_state.peakCpuUsage.store(percent, std::memory_order_relaxed);
        }
    }

    void TelemetryManager::PublishInputLevel(float rms) {
        g_state.inputRmsLevel.store(rms, std::memory_order_relaxed);
    }

    void TelemetryManager::PublishOutputLevel(float rms) {
        g_state.outputRmsLevel.store(rms, std::memory_order_relaxed);
    }

    void TelemetryManager::IncrementXrun() {
        g_state.xrunCount.fetch_add(1, std::memory_order_relaxed);
    }

    TelemetryData TelemetryManager::GetSnapshot() const {
        TelemetryData snap;
        snap.currentLatencyMs = g_state.currentLatencyMs.load(std::memory_order_relaxed);
        snap.peakLatencyMs = g_state.peakLatencyMs.load(std::memory_order_relaxed);
        snap.currentCpuUsage = g_state.currentCpuUsage.load(std::memory_order_relaxed);
        snap.peakCpuUsage = g_state.peakCpuUsage.load(std::memory_order_relaxed);
        snap.inferenceTimeMs = g_state.inferenceTimeMs.load(std::memory_order_relaxed);
        snap.inputRmsLevel = g_state.inputRmsLevel.load(std::memory_order_relaxed);
        snap.outputRmsLevel = g_state.outputRmsLevel.load(std::memory_order_relaxed);
        snap.activeSampleRate = g_state.activeSampleRate.load(std::memory_order_relaxed);
        snap.xrunCount = g_state.xrunCount.load(std::memory_order_relaxed);
        snap.droppedFrames = g_state.droppedFrames.load(std::memory_order_relaxed);
        return snap;
    }

    void TelemetryManager::Reset() noexcept {
        g_state.currentLatencyMs.store(0.0f, std::memory_order_relaxed);
        g_state.peakLatencyMs.store(0.0f,    std::memory_order_relaxed);
        g_state.currentCpuUsage.store(0.0f,  std::memory_order_relaxed);
        g_state.peakCpuUsage.store(0.0f,     std::memory_order_relaxed);
        g_state.inferenceTimeMs.store(0.0f,  std::memory_order_relaxed);
        g_state.inputRmsLevel.store(0.0f,    std::memory_order_relaxed);
        g_state.outputRmsLevel.store(0.0f,   std::memory_order_relaxed);
        g_state.xrunCount.store(0,           std::memory_order_relaxed);
        g_state.droppedFrames.store(0,       std::memory_order_relaxed);
    }

}

