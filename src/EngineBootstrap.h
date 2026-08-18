#pragma once
/**
 * @file EngineBootstrap.h
 * @brief Root composition root for all engine subsystems.
 *
 * Changes vs. previous version:
 *  - m_inferenceWorker replaces direct InferenceEngine embedding.
 *    The InferenceWorker runs ONNX on its own MMCSS-boosted thread,
 *    decoupling inference latency from the capture ring buffer.
 *  - ReloadModel() added for hot-swapping the ONNX model at runtime.
 *  - GetActiveProfileName() / GetActiveModelName() added for telemetry.
 */

#include <memory>
#include <atomic>
#include <string>
#include <mutex>
#include "audio/AudioPipeline.h"
#include "audio/CaptureEngine.h"
#include "audio/OutputEngine.h"
#include "driver/DriverBridge.h"
#include "ai/ModelManager.h"
#include "ai/InferenceWorker.h"
#include "ai/NoiseProfile.h"
#include "diagnostics/TelemetryManager.h"
#include "core/ipc/IpcProtocol.h"

namespace VoiceClear {

    class EngineBootstrap {
    public:
        EngineBootstrap();
        ~EngineBootstrap();

        bool Initialize();
        bool Start();
        void Stop();

        // ----------------------------------------------------------------
        // Control API (called from IpcServer / Watchdog)
        // ----------------------------------------------------------------

        /// Returns 0=Unknown, 1=Real (AVStream), 2=Mock
        uint8_t     GetDriverMode() const;

        bool        IsAiEnabled()    const;
        void        SetAiEnabled(bool enabled);

        /// Sets the named noise profile on the active InferenceWorker.
        void        SetProfile(const char* profileId);

        /**
         * @brief Hot-reloads the ONNX model while the engine is running.
         * Temporarily pauses the AI pipeline, loads the new session, and
         * resumes.  Returns false if the load fails (keeps the old model).
         */
        bool        ReloadModel(const char* modelName);

        // ----------------------------------------------------------------
        // Telemetry helpers
        // ----------------------------------------------------------------

        /// Null-terminated name of the currently active NoiseProfile.
        const char* GetActiveProfileName() const noexcept;

        /// Null-terminated name of the currently loaded ONNX model file.
        const char* GetActiveModelName()   const noexcept;

        // ----------------------------------------------------------------
        // Device Management API
        // ----------------------------------------------------------------

        /// Fills IPC payload with all available input and output devices.
        IPC::DeviceListPayload GetDeviceListPayload();

        /// Switches input microphone and/or output render endpoint.
        bool SetAudioDevices(const std::string& inputId, const std::string& outputId);

        /// Audio Monitor ("Hear Myself") controls
        void SetMonitorEnabled(bool enabled);
        bool IsMonitorEnabled() const;

        /// Friendly names for telemetry / UI display
        const char* GetCurrentInputDeviceName()  const noexcept;
        const char* GetCurrentOutputDeviceName() const noexcept;

    private:
        // Audio subsystems
        std::shared_ptr<Audio::DeviceManager>               m_deviceManager;
        std::shared_ptr<Audio::CaptureEngine>               m_captureEngine;
        std::shared_ptr<Audio::OutputEngine>                m_outputEngine;
        std::shared_ptr<Audio::AudioPipeline>               m_audioPipeline;
        std::shared_ptr<Audio::LockFreeRingBuffer<float>>   m_captureToPipelineBuffer;
        std::shared_ptr<Audio::LockFreeRingBuffer<float>>   m_pipelineToOutputBuffer;

        // AI subsystems
        std::shared_ptr<AI::ModelManager>                   m_modelManager;
        std::shared_ptr<AI::InferenceWorker>                m_inferenceWorker;

        // Driver
        std::shared_ptr<Driver::IDriverBridge>              m_driverBridge;

        // Diagnostics
        std::shared_ptr<Diagnostics::TelemetryManager>      m_telemetry;

        // State
        std::atomic<bool>  m_aiEnabled{true};
        std::atomic<bool>  m_isPrerolling{true};
        std::string        m_activeProfile{"Balanced"};
        std::string        m_activeModel{"deepfilternet3.onnx"};
        std::string        m_selectedInputId{"default"};
        std::string        m_selectedInputName{"Default Microphone"};
        std::string        m_selectedOutputId{"default"};
        std::string        m_selectedOutputName{"CABLE Input (VB-Audio Virtual Cable)"};
        mutable std::mutex m_modelReloadMutex;
        mutable std::mutex m_deviceMutex;
    };

} // namespace VoiceClear
