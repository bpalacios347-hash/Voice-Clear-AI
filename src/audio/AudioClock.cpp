#include "AudioClock.h"
#include "../profiling/ProfilingMacros.h"
#include <windows.h>
#include <cmath>
#include <algorithm>

namespace VoiceClear::Audio {

    // Cached QPC frequency to avoid expensive repeated queries
    static uint64_t GetQpcFrequency() {
        static uint64_t freq = 0;
        if (freq == 0) {
            LARGE_INTEGER li;
            QueryPerformanceFrequency(&li);
            freq = li.QuadPart;
        }
        return freq;
    }

    AudioClock::AudioClock(std::shared_ptr<Diagnostics::TelemetryManager> telemetry)
        : m_telemetry(std::move(telemetry)) {
        Reset();
    }

    uint64_t AudioClock::Now() noexcept {
        LARGE_INTEGER li;
        QueryPerformanceCounter(&li);
        return static_cast<uint64_t>(li.QuadPart);
    }

    double AudioClock::TicksToMs(uint64_t ticks) noexcept {
        return (static_cast<double>(ticks) * 1000.0) / static_cast<double>(GetQpcFrequency());
    }

    void AudioClock::Reset() noexcept {
        m_totalBlocks.store(0, std::memory_order_relaxed);
        m_avgLatencyMs.store(0.0, std::memory_order_relaxed);
        m_peakLatencyMs.store(0.0, std::memory_order_relaxed);
        m_maxJitterMs.store(0.0, std::memory_order_relaxed);
        m_jitterSum.store(0.0, std::memory_order_relaxed);
        m_lastRenderTick.store(0, std::memory_order_relaxed);
        m_firstCaptureTick.store(0, std::memory_order_relaxed);
        m_firstRenderTick.store(0, std::memory_order_relaxed);
        m_driftMs.store(0.0, std::memory_order_relaxed);
    }

    void AudioClock::RecordCaptureTimestamp(AudioBuffer& buffer) const noexcept {
        buffer.metadata.captureTimestamp = Now();
    }

    void AudioClock::RecordRingBufferEntry(AudioBuffer& buffer) const noexcept {
        buffer.metadata.ringBufferEntryTimestamp = Now();
    }

    void AudioClock::RecordRingBufferExit(AudioBuffer& buffer) const noexcept {
        buffer.metadata.ringBufferExitTimestamp = Now();
    }

    void AudioClock::RecordRenderTimestamp(AudioBuffer& buffer) const noexcept {
        buffer.metadata.renderTimestamp = Now();
    }

    void AudioClock::AnalyzeBlock(const AudioBuffer& buffer) noexcept {
        VC_PROFILE_ZONE("AudioClock_AnalyzeBlock");

        if (buffer.metadata.captureTimestamp == 0 || buffer.metadata.renderTimestamp == 0) return;

        double e2eLatencyMs = TicksToMs(buffer.metadata.renderTimestamp - buffer.metadata.captureTimestamp);

        // Update Peak Latency (Atomic Compare-and-Swap idiom for doubles)
        double currentPeak = m_peakLatencyMs.load(std::memory_order_relaxed);
        while (e2eLatencyMs > currentPeak && 
               !m_peakLatencyMs.compare_exchange_weak(currentPeak, e2eLatencyMs, std::memory_order_relaxed)) {}

        // Welford's running average
        uint64_t n = m_totalBlocks.fetch_add(1, std::memory_order_relaxed) + 1;
        double currentAvg = m_avgLatencyMs.load(std::memory_order_relaxed);
        double newAvg = currentAvg + (e2eLatencyMs - currentAvg) / n;
        m_avgLatencyMs.store(newAvg, std::memory_order_relaxed);

        // Jitter Calculation (difference in interval vs expected block duration)
        uint64_t lastRender = m_lastRenderTick.load(std::memory_order_relaxed);
        if (lastRender != 0) {
            double intervalMs = TicksToMs(buffer.metadata.renderTimestamp - lastRender);
            double expectedMs = (static_cast<double>(buffer.frames) * 1000.0) / buffer.sampleRate;
            double jitter = std::abs(intervalMs - expectedMs);
            
            double currentMaxJitter = m_maxJitterMs.load(std::memory_order_relaxed);
            while (jitter > currentMaxJitter && 
                   !m_maxJitterMs.compare_exchange_weak(currentMaxJitter, jitter, std::memory_order_relaxed)) {}
            
            // Running sum for variance proxy
            double currentJitterSum = m_jitterSum.load(std::memory_order_relaxed);
            m_jitterSum.store(currentJitterSum + jitter, std::memory_order_relaxed);
        }
        m_lastRenderTick.store(buffer.metadata.renderTimestamp, std::memory_order_relaxed);

        // Clock drift (Capture clock vs Render clock divergence over time)
        if (n == 1) {
            m_firstCaptureTick.store(buffer.metadata.captureTimestamp, std::memory_order_relaxed);
            m_firstRenderTick.store(buffer.metadata.renderTimestamp, std::memory_order_relaxed);
        } else {
            uint64_t firstCapture = m_firstCaptureTick.load(std::memory_order_relaxed);
            uint64_t firstRender = m_firstRenderTick.load(std::memory_order_relaxed);
            
            double elapsedCaptureMs = TicksToMs(buffer.metadata.captureTimestamp - firstCapture);
            double elapsedRenderMs = TicksToMs(buffer.metadata.renderTimestamp - firstRender);
            
            m_driftMs.store(elapsedCaptureMs - elapsedRenderMs, std::memory_order_relaxed);
        }

        // Publish to UI via telemetry
        if (m_telemetry) {
            m_telemetry->PublishLatency(static_cast<float>(e2eLatencyMs));
        }
    }

    ClockStatistics AudioClock::GetStatistics() const noexcept {
        ClockStatistics stats;
        stats.averageLatencyMs = m_avgLatencyMs.load(std::memory_order_relaxed);
        stats.peakLatencyMs = m_peakLatencyMs.load(std::memory_order_relaxed);
        stats.maxJitterMs = m_maxJitterMs.load(std::memory_order_relaxed);
        stats.driftMs = m_driftMs.load(std::memory_order_relaxed);
        
        uint64_t n = m_totalBlocks.load(std::memory_order_relaxed);
        if (n > 1) {
            stats.jitterVarianceMs = m_jitterSum.load(std::memory_order_relaxed) / static_cast<double>(n - 1);
        }
        return stats;
    }

}
