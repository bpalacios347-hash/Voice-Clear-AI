#pragma once
#include <string>
#include <thread>
#include <atomic>
#include <functional>
#include <memory>
#include "DeviceManager.h"
#include "LockFreeRingBuffer.h"
#include "../diagnostics/TelemetryManager.h"

namespace VoiceClear::Audio {

    struct OutputConfig {
        std::string deviceId;
        bool preferExclusive{true};
        size_t requestedBufferFrames{256};
    };

    /**
     * @brief High-performance WASAPI Output Engine.
     * Handles Shared/Exclusive mode rendering, priority boosting, and robust underrun/overrun recovery.
     */
    class OutputEngine {
    public:
        // Callback to request audio data from the pipeline (typically pops from LockFreeRingBuffer)
        using AudioDataRequestCallback = std::function<size_t(float* outData, size_t requestedFrames, int channels, int sampleRate)>;

        OutputEngine(std::shared_ptr<Diagnostics::TelemetryManager> telemetry);
        ~OutputEngine();

        bool Initialize(const OutputConfig& config);
        bool Start();
        void Stop();
        void Shutdown();

        // Register the callback to fetch audio from the lock-free pipeline
        void SetAudioRequestCallback(AudioDataRequestCallback callback) { m_callback = callback; }

        bool IsExclusiveMode() const { return m_isExclusive; }
        int GetSampleRate() const { return m_sampleRate; }
        int GetChannels() const { return m_channels; }

        void SetMonitorEnabled(bool enabled) { m_monitorEnabled.store(enabled, std::memory_order_relaxed); }
        bool IsMonitorEnabled() const { return m_monitorEnabled.load(std::memory_order_relaxed); }

    private:
        void RenderThreadLoop();
        bool InitializeWasapi(bool requestExclusive);
        
        std::shared_ptr<Diagnostics::TelemetryManager> m_telemetry;
        AudioDataRequestCallback m_callback;

        std::string m_deviceId;
        bool m_isExclusive{false};
        std::atomic<bool> m_monitorEnabled{false};
        int m_sampleRate{48000};
        int m_channels{1};

        std::thread m_renderThread;
        std::atomic<bool> m_running{false};
        std::atomic<bool> m_initialized{false};

        // COM Interfaces managed strictly on the render thread
        void* m_pAudioClient{nullptr};
        void* m_pRenderClient{nullptr};
        void* m_hEvent{nullptr};

        std::unique_ptr<LockFreeRingBuffer<float>> m_monitorBuffer;
    };

}
