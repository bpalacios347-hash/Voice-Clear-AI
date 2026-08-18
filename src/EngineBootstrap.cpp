/**
 * @file EngineBootstrap.cpp
 * @brief Composition root — wires all engine subsystems together.
 *
 * Audio data flow:
 *
 *   WASAPI Mic
 *       │  (callback)
 *       ▼
 *   CaptureEngine  ──push──►  captureToPipelineBuffer (SPSC ring, 48k floats)
 *                                      │
 *                             AudioPipeline Processing Thread  ──pop──►
 *                                      │  (DSP pass-through stages)
 *                             pipelineToInferenceBuffer (SPSC ring, 48k floats)
 *                                      │
 *                             InferenceWorker (dedicated MMCSS thread)
 *                                      │  ONNX inference per 480-sample frame
 *                             inferenceToOutputBuffer (SPSC ring, 48k floats)
 *                                      │
 *   OutputEngine  ◄──pop──────────────┘
 *       │
 *       ▼
 *   RealDriverBridge  ──►  Shared Memory  ──►  VoiceClearVAD.sys (kernel)
 */

#include "EngineBootstrap.h"
#include "audio/DspStages.h"
#include "ai/DeepFilterNetProcessor.h"
#include "utils/Logger.h"
#include <algorithm>

namespace VoiceClear {

    // =========================================================================
    // Construction
    // =========================================================================

    EngineBootstrap::EngineBootstrap() {
        m_telemetry = std::make_shared<Diagnostics::TelemetryManager>();

        // 1 second @ 48 kHz stereo — generous headroom for jitter
        m_captureToPipelineBuffer = std::make_shared<Audio::LockFreeRingBuffer<float>>(48000 * 2);
        m_pipelineToOutputBuffer  = std::make_shared<Audio::LockFreeRingBuffer<float>>(48000 * 2);
    }

    EngineBootstrap::~EngineBootstrap() { Stop(); }

    // =========================================================================
    // Initialize
    // =========================================================================

    bool EngineBootstrap::Initialize() {
        Utils::Logger::GetInstance()->Log(Core::LogLevel::Info,
            "[EngineBootstrap] Initializing Voice Clear AI Engine...");

        // --- Driver Bridge ---
        m_driverBridge = std::make_shared<Driver::RealDriverBridge>(m_telemetry);
        if (!m_driverBridge->Connect()) {
            Utils::Logger::GetInstance()->Log(Core::LogLevel::Warn,
                "[EngineBootstrap] RealDriverBridge unavailable — "
                "audio output will be dropped until driver is installed.");
            // Non-fatal: engine can still process audio for diagnostics purposes
        }

        // --- Model Manager + Initial ONNX Session ---
        m_modelManager = std::make_shared<AI::ModelManager>(m_telemetry);
        auto session = m_modelManager->LoadModel(m_activeModel);
        if (!session) {
            Utils::Logger::GetInstance()->Log(Core::LogLevel::Error,
                "[EngineBootstrap] Failed to load ONNX model: " + m_activeModel);
            return false;
        }

        // --- Build DeepFilterNetProcessor ---
        auto dfnProcessor = std::make_unique<AI::DeepFilterNetProcessor>(m_telemetry);
        if (!dfnProcessor->Initialize(std::move(session))) {
            Utils::Logger::GetInstance()->Log(Core::LogLevel::Error,
                "[EngineBootstrap] DeepFilterNetProcessor initialization failed.");
            return false;
        }

        // --- InferenceWorker (owns its own dedicated ONNX thread) ---
        //
        // The InferenceWorker pops from captureToPipelineBuffer and pushes into
        // pipelineToOutputBuffer, completely bypassing the AudioPipeline for
        // inference-heavy work.  The AudioPipeline is kept for lightweight
        // pre/post DSP stages that are safe to run on the capture thread.
        m_inferenceWorker = std::make_shared<AI::InferenceWorker>(
            m_captureToPipelineBuffer,
            m_pipelineToOutputBuffer,
            m_telemetry);
        m_inferenceWorker->SetProcessor(std::move(dfnProcessor));

        // --- Capture Engine ---
        m_captureEngine = std::make_shared<Audio::CaptureEngine>(m_telemetry);
        m_captureEngine->SetAudioCallback(
            [this](const float* data, size_t frames, int channels, int sampleRate) {
                if (!data || frames == 0) return;

                // 1. Extract channel 0 as mono for the AI pipeline
                //    Most USB microphones/headsets report stereo but only have signal on ch0.
                //    Averaging with a silent ch1 would lose -6 dB. Always use ch0.
                std::vector<float> monoBuf(frames);
                if (channels == 1) {
                    std::copy(data, data + frames, monoBuf.begin());
                } else {
                    // Extract channel 0 directly (no averaging, no volume loss)
                    for (size_t i = 0; i < frames; ++i) {
                        monoBuf[i] = data[i * channels];
                    }
                }

                // 2. High-quality linear resampler if microphone sample rate differs from 48000 Hz
                if (sampleRate > 0 && sampleRate != 48000) {
                    double ratio = 48000.0 / static_cast<double>(sampleRate);
                    size_t outFrames = static_cast<size_t>(std::round(frames * ratio));
                    if (outFrames > 0) {
                        std::vector<float> resampled(outFrames);
                        for (size_t i = 0; i < outFrames; ++i) {
                            double srcPos = i / ratio;
                            size_t idx0 = static_cast<size_t>(srcPos);
                            size_t idx1 = std::min(idx0 + 1, frames - 1);
                            float frac = static_cast<float>(srcPos - idx0);
                            resampled[i] = (1.0f - frac) * monoBuf[idx0] + frac * monoBuf[idx1];
                        }
                        monoBuf = std::move(resampled);
                        frames = monoBuf.size();
                    }
                }

                if (m_aiEnabled.load(std::memory_order_relaxed)) {
                    m_captureToPipelineBuffer->Push(monoBuf.data(), frames);
                    if (m_inferenceWorker) {
                        m_inferenceWorker->NotifyAudioAvailable();
                    }
                } else {
                    m_pipelineToOutputBuffer->Push(monoBuf.data(), frames);
                }
            });

        // --- Output Engine ---
        m_outputEngine = std::make_shared<Audio::OutputEngine>(m_telemetry);
        m_outputEngine->SetAudioRequestCallback(
            [this](float* outData, size_t requestedFrames,
                   int channels, int /*sampleRate*/) -> size_t {
                if (!outData || requestedFrames == 0) return 0;

                // 1. Initial Pre-Roll Protection (Stream startup cushion: 960 samples = 20ms)
                size_t avail = m_pipelineToOutputBuffer->ReadAvailable();

                if (m_isPrerolling.load(std::memory_order_relaxed)) {
                    if (avail < 960) {
                        // Still pre-buffering on initial start: fill WASAPI with clean silence
                        std::fill(outData, outData + (requestedFrames * channels), 0.0f);
                        return requestedFrames;
                    }
                    m_isPrerolling.store(false, std::memory_order_relaxed);
                }

                // 2. Prevent Buffer Overflow / Latency Drift (Cap max delay at ~80ms = 3840 samples)
                if (avail > 3840) {
                    size_t toDrop = avail - 1920;
                    std::vector<float> dropBuf(toDrop);
                    m_pipelineToOutputBuffer->Pop(dropBuf.data(), toDrop);
                    avail = m_pipelineToOutputBuffer->ReadAvailable();
                }

                // 3. Pop available audio with smooth Packet Loss Concealment (PLC)
                std::vector<float> monoBuf(requestedFrames, 0.0f);
                size_t got = m_pipelineToOutputBuffer->Pop(monoBuf.data(), requestedFrames);

                static float s_lastSample = 0.0f;

                if (got < requestedFrames) {
                    if (got > 0) {
                        // Smoothly decay the tail to avoid abrupt clicks
                        for (size_t i = got; i < requestedFrames; ++i) {
                            monoBuf[i] = monoBuf[got - 1] * std::pow(0.92f, static_cast<float>(i - got + 1));
                        }
                    } else {
                        // PLC: Soft exponential continuation from last sample rather than harsh zero-gap
                        for (size_t i = 0; i < requestedFrames; ++i) {
                            monoBuf[i] = s_lastSample * std::pow(0.85f, static_cast<float>(i + 1));
                        }
                    }
                }
                s_lastSample = monoBuf[requestedFrames - 1];

                if (channels == 1) {
                    std::copy(monoBuf.begin(), monoBuf.end(), outData);
                } else if (channels == 2) {
                    for (size_t i = 0; i < requestedFrames; ++i) {
                        outData[2 * i]     = monoBuf[i];
                        outData[2 * i + 1] = monoBuf[i];
                    }
                } else {
                    for (size_t i = 0; i < requestedFrames; ++i) {
                        for (int c = 0; c < channels; ++c) {
                            outData[i * channels + c] = (c < 2) ? monoBuf[i] : 0.0f;
                        }
                    }
                }

                return requestedFrames;
            });

        Utils::Logger::GetInstance()->Log(Core::LogLevel::Info,
            "[EngineBootstrap] Initialization complete.");
        return true;
    }

    // =========================================================================
    // Start / Stop
    // =========================================================================

    bool EngineBootstrap::Start() {
        // Start InferenceWorker first so its ring is ready before capture fires
        if (!m_inferenceWorker->Start()) return false;

        std::string targetInput = m_selectedInputId.empty() ? "default" : m_selectedInputId;
        Audio::CaptureConfig capConfig{targetInput, false, 256};
        if (!m_captureEngine->Initialize(capConfig) || !m_captureEngine->Start()) {
            m_inferenceWorker->Stop();
            return false;
        }

        std::string targetOutput = m_selectedOutputId.empty() ? "default" : m_selectedOutputId;
        if (targetOutput == "default") {
            if (!m_deviceManager) {
                m_deviceManager = std::make_shared<Audio::DeviceManager>();
                m_deviceManager->Initialize();
            }
            auto outDevs = m_deviceManager->EnumerateOutputDevices();
            for (const auto& dev : outDevs) {
                if (dev.friendlyName.find("CABLE") != std::string::npos ||
                    dev.friendlyName.find("Cable") != std::string::npos) {
                    targetOutput = dev.id;
                    m_selectedOutputId = dev.id;
                    m_selectedOutputName = dev.friendlyName;
                    break;
                }
            }
        }

        Audio::OutputConfig outConfig{targetOutput, false, 256};
        if (!m_outputEngine->Initialize(outConfig) || !m_outputEngine->Start()) {
            m_captureEngine->Stop();
            m_inferenceWorker->Stop();
            return false;
        }

        Utils::Logger::GetInstance()->Log(Core::LogLevel::Info,
            "[EngineBootstrap] Engine started with output target: " + m_selectedOutputName);
        return true;
    }

    void EngineBootstrap::Stop() {
        if (m_captureEngine)    m_captureEngine->Stop();
        if (m_outputEngine)     m_outputEngine->Stop();
        if (m_inferenceWorker)  m_inferenceWorker->Stop();
        if (m_driverBridge)     m_driverBridge->Disconnect();

        Utils::Logger::GetInstance()->Log(Core::LogLevel::Info,
            "[EngineBootstrap] Engine stopped.");
    }

    // =========================================================================
    // Control API
    // =========================================================================

    uint8_t EngineBootstrap::GetDriverMode() const {
        if (!m_driverBridge || !m_driverBridge->IsConnected()) return 0;
        // RealDriverBridge is always mode 1; MockDriverBridge would return 2
        return 1;
    }

    bool EngineBootstrap::IsAiEnabled() const {
        return m_aiEnabled.load(std::memory_order_relaxed);
    }

    void EngineBootstrap::SetAiEnabled(bool enabled) {
        m_aiEnabled.store(enabled, std::memory_order_relaxed);

        if (m_inferenceWorker) {
            if (!enabled) {
                m_inferenceWorker->SetProfile(AI::NoiseProfile::Bypass());
            } else {
                std::string active;
                {
                    std::lock_guard<std::mutex> lk(m_modelReloadMutex);
                    active = m_activeProfile;
                }
                if (active.empty()) active = "Balanced";
                SetProfile(active.c_str());
            }
        }

        Utils::Logger::GetInstance()->Log(Core::LogLevel::Info,
            std::string("[EngineBootstrap] AI processing ") +
            (enabled ? "enabled." : "disabled (bypass)."));
    }

    void EngineBootstrap::SetProfile(const char* profileId) {
        if (!m_inferenceWorker || !profileId) return;

        std::string id(profileId);
        AI::NoiseProfile profile = AI::NoiseProfile::Balanced();

        if      (id == "Strong")             profile = AI::NoiseProfile::Strong();
        else if (id == "Voice Preservation") profile = AI::NoiseProfile::VoicePreservation();
        else if (id == "Low CPU")            profile = AI::NoiseProfile::LowCpu();
        else if (id == "Bypass")             profile = AI::NoiseProfile::Bypass();

        m_inferenceWorker->SetProfile(profile);

        {
            std::lock_guard<std::mutex> lk(m_modelReloadMutex);
            m_activeProfile = profile.name;
        }

        Utils::Logger::GetInstance()->Log(Core::LogLevel::Info,
            "[EngineBootstrap] Profile switched to: " + profile.name);
    }

    bool EngineBootstrap::ReloadModel(const char* modelName) {
        if (!modelName) return false;

        std::lock_guard<std::mutex> lk(m_modelReloadMutex);

        Utils::Logger::GetInstance()->Log(Core::LogLevel::Info,
            "[EngineBootstrap] Hot-reloading model: " + std::string(modelName));

        // Load new session (may take several hundred ms — done off the RT thread)
        auto newSession = m_modelManager->LoadModel(modelName);
        if (!newSession) {
            Utils::Logger::GetInstance()->Log(Core::LogLevel::Error,
                "[EngineBootstrap] ReloadModel failed for: " + std::string(modelName));
            return false;
        }

        // Build a fresh processor
        auto newProcessor = std::make_unique<AI::DeepFilterNetProcessor>(m_telemetry);
        if (!newProcessor->Initialize(std::move(newSession))) {
            Utils::Logger::GetInstance()->Log(Core::LogLevel::Error,
                "[EngineBootstrap] Processor init failed for new model.");
            return false;
        }

        // Pause inference, swap processor, resume
        if (m_inferenceWorker) {
            m_inferenceWorker->Stop();
            m_inferenceWorker->SetProcessor(std::move(newProcessor));
            m_inferenceWorker->Start();
        }

        m_activeModel = modelName;

        Utils::Logger::GetInstance()->Log(Core::LogLevel::Info,
            "[EngineBootstrap] Model hot-reloaded successfully: " + m_activeModel);
        return true;
    }

    // =========================================================================
    // Telemetry & Device Helpers
    // =========================================================================

    const char* EngineBootstrap::GetActiveProfileName() const noexcept {
        return m_activeProfile.c_str();
    }

    const char* EngineBootstrap::GetActiveModelName() const noexcept {
        return m_activeModel.c_str();
    }

    const char* EngineBootstrap::GetCurrentInputDeviceName() const noexcept {
        return m_selectedInputName.c_str();
    }

    const char* EngineBootstrap::GetCurrentOutputDeviceName() const noexcept {
        return m_selectedOutputName.c_str();
    }

    IPC::DeviceListPayload EngineBootstrap::GetDeviceListPayload() {
        std::lock_guard<std::mutex> lk(m_deviceMutex);
        IPC::DeviceListPayload payload{};
        uint32_t count = 0;

        if (!m_deviceManager) {
            m_deviceManager = std::make_shared<Audio::DeviceManager>();
            m_deviceManager->Initialize();
        }

        // 1. Enumerate inputs (microphones)
        auto inputs = m_deviceManager->EnumerateInputDevices();
        std::string fallbackPhysicalInputId = "";
        std::string fallbackPhysicalInputName = "";

        for (const auto& dev : inputs) {
            if (count >= 32) break;
            if (dev.state != Audio::DeviceState::Active) continue;
            auto& entry = payload.devices[count++];
            strncpy_s(entry.id, dev.id.c_str(), sizeof(entry.id) - 1);
            strncpy_s(entry.friendlyName, dev.friendlyName.c_str(), sizeof(entry.friendlyName) - 1);
            entry.isCapture = 1;
            entry.isDefault = dev.isDefaultConsole ? 1 : 0;

            bool isVirtual = (dev.friendlyName.find("CABLE") != std::string::npos ||
                              dev.friendlyName.find("Cable") != std::string::npos ||
                              dev.friendlyName.find("VB-Audio") != std::string::npos ||
                              dev.friendlyName.find("Stereo Mix") != std::string::npos ||
                              dev.friendlyName.find("Mezcla") != std::string::npos);

            if (!isVirtual && fallbackPhysicalInputId.empty()) {
                fallbackPhysicalInputId = dev.id;
                fallbackPhysicalInputName = dev.friendlyName;
            }

            if (m_selectedInputId == "default" && !isVirtual && m_selectedInputName.empty()) {
                m_selectedInputId = dev.id;
                m_selectedInputName = dev.friendlyName;
            }
            entry.isSelected = (dev.id == m_selectedInputId) ? 1 : 0;
            if (entry.isSelected) {
                m_selectedInputName = dev.friendlyName;
            }
        }

        if (m_selectedInputId == "default" || m_selectedInputName.find("CABLE") != std::string::npos || m_selectedInputName.find("Stereo Mix") != std::string::npos) {
            if (!fallbackPhysicalInputId.empty()) {
                m_selectedInputId = fallbackPhysicalInputId;
                m_selectedInputName = fallbackPhysicalInputName;
                for (uint32_t i = 0; i < count; ++i) {
                    if (payload.devices[i].isCapture) {
                        payload.devices[i].isSelected = (payload.devices[i].id == m_selectedInputId) ? 1 : 0;
                    }
                }
            }
        }

        // 2. Enumerate outputs (speakers / VB-Cable input)
        auto outputs = m_deviceManager->EnumerateOutputDevices();
        for (const auto& dev : outputs) {
            if (count >= 32) break;
            if (dev.state != Audio::DeviceState::Active) continue;
            auto& entry = payload.devices[count++];
            strncpy_s(entry.id, dev.id.c_str(), sizeof(entry.id) - 1);
            strncpy_s(entry.friendlyName, dev.friendlyName.c_str(), sizeof(entry.friendlyName) - 1);
            entry.isCapture = 0;
            entry.isDefault = dev.isDefaultConsole ? 1 : 0;
            
            // Prefer CABLE Input by default if available
            bool isCable = (dev.friendlyName.find("CABLE") != std::string::npos ||
                            dev.friendlyName.find("Cable") != std::string::npos);
            if (m_selectedOutputId == "default" && isCable) {
                m_selectedOutputId = dev.id;
                m_selectedOutputName = dev.friendlyName;
            }
            entry.isSelected = (dev.id == m_selectedOutputId) ? 1 : 0;
        }

        payload.deviceCount = count;
        strncpy_s(payload.selectedInputId, m_selectedInputId.c_str(), sizeof(payload.selectedInputId) - 1);
        strncpy_s(payload.selectedOutputId, m_selectedOutputId.c_str(), sizeof(payload.selectedOutputId) - 1);
        return payload;
    }

    bool EngineBootstrap::SetAudioDevices(const std::string& inputId, const std::string& outputId) {
        std::lock_guard<std::mutex> lk(m_deviceMutex);
        Utils::Logger::GetInstance()->Log(Core::LogLevel::Info,
            "[EngineBootstrap] Switching devices -> Input: " + inputId + " | Output: " + outputId);

        bool inChanged = (!inputId.empty() && inputId != m_selectedInputId);
        bool outChanged = (!outputId.empty() && outputId != m_selectedOutputId);

        if (inChanged) {
            m_selectedInputId = inputId;
            if (m_deviceManager) {
                for (const auto& dev : m_deviceManager->EnumerateInputDevices()) {
                    if (dev.id == inputId) {
                        m_selectedInputName = dev.friendlyName;
                        break;
                    }
                }
            }
            if (m_captureEngine) {
                m_captureEngine->Stop();
                m_captureEngine->Initialize(Audio::CaptureConfig{m_selectedInputId, false, 256});
                m_captureEngine->Start();
            }
        }

        if (outChanged) {
            m_selectedOutputId = outputId;
            if (m_deviceManager) {
                for (const auto& dev : m_deviceManager->EnumerateOutputDevices()) {
                    if (dev.id == outputId) {
                        m_selectedOutputName = dev.friendlyName;
                        break;
                    }
                }
            }
            if (m_outputEngine) {
                m_outputEngine->Stop();
                m_outputEngine->Initialize(Audio::OutputConfig{m_selectedOutputId, false, 256});
                m_outputEngine->Start();
            }
        }

        return true;
    }

    void EngineBootstrap::SetMonitorEnabled(bool enabled) {
        if (m_outputEngine) {
            m_outputEngine->SetMonitorEnabled(enabled);
        }
    }

    bool EngineBootstrap::IsMonitorEnabled() const {
        return m_outputEngine ? m_outputEngine->IsMonitorEnabled() : false;
    }

} // namespace VoiceClear
