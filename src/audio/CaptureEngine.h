#pragma once
#include <string>
#include <thread>
#include <atomic>
#include <functional>
#include "DeviceManager.h"
#include "interfaces/IAudioStage.h"
#include "../diagnostics/TelemetryManager.h"

namespace VoiceClear::Audio {

    struct CaptureConfig {
        std::string deviceId;
        bool preferExclusive{true};
        size_t requestedBufferFrames{256};
    };

    /**
     * @brief High-performance WASAPI Capture Engine.
     * Handles Shared/Exclusive mode, thread priority boosting, and lock-free telemetry.
     */
    class CaptureEngine {
    public:
        using AudioDataCallback = std::function<void(const float* data, size_t frames, int channels, int sampleRate)>;

        CaptureEngine(std::shared_ptr<Diagnostics::TelemetryManager> telemetry);
        ~CaptureEngine();

        bool Initialize(const CaptureConfig& config);
        bool Start();
        void Stop();
        void Shutdown();

        // Register the callback to push captured audio into the pipeline
        void SetAudioCallback(AudioDataCallback callback) { m_callback = callback; }

        bool IsExclusiveMode() const { return m_isExclusive; }
        int GetSampleRate() const { return m_sampleRate; }
        int GetChannels() const { return m_channels; }

    private:
        void CaptureThreadLoop();
        bool InitializeWasapi(bool requestExclusive);
        
        std::shared_ptr<Diagnostics::TelemetryManager> m_telemetry;
        AudioDataCallback m_callback;

        std::string m_deviceId;
        bool m_isExclusive{false};
        int m_sampleRate{48000};
        int m_channels{1};

        std::thread m_captureThread;
        std::atomic<bool> m_running{false};
        std::atomic<bool> m_initialized{false};

        // COM Interfaces (Pointers strictly managed within the capture thread for affinity if needed)
        void* m_pAudioClient{nullptr};
        void* m_pCaptureClient{nullptr};
        void* m_hEvent{nullptr};
    };

}
