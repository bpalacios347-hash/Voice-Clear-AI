#pragma once
#include "IAudioProcessor.h"
#include "InferenceContext.h"
#include "NoiseProfile.h"
#include "../diagnostics/TelemetryManager.h"
#include "../dsp/STFT.h"
#include "../dsp/ErbFeatures.h"
#include <memory>
#include <atomic>
#include <onnxruntime_cxx_api.h>

namespace VoiceClear::AI {

    /**
     * @brief DeepFilterNet processing node. 
     * Orchestrates ONNX inference and Adaptive Spectral Wiener Filtering.
     */
    class DeepFilterNetProcessor : public IAudioProcessor {
    public:
        explicit DeepFilterNetProcessor(std::shared_ptr<Diagnostics::TelemetryManager> telemetry);
        ~DeepFilterNetProcessor() override = default;

        /**
         * @brief Injects the active session provided by the ModelManager and allocates the context.
         */
        bool Initialize(std::unique_ptr<Ort::Session> session);

        void ResetState() noexcept override;
        
        /**
         * @brief Executes ONNX inference & Adaptive Wiener Filtering on the buffer. Zero-allocations.
         */
        void Process(Audio::AudioBuffer& buffer) noexcept override;

        /**
         * @brief Asynchronously sets the target noise profile.
         */
        void SetProfile(const NoiseProfile& profile) noexcept;

    private:
        std::shared_ptr<Diagnostics::TelemetryManager> m_telemetry;
        std::unique_ptr<Ort::Session> m_session;
        std::unique_ptr<InferenceContext> m_context;

        std::unique_ptr<DSP::STFT> m_stft;
        std::unique_ptr<DSP::ErbFeatures> m_erb;

        // Double-buffered real-time profile settings
        DSP::DspProfileSettings m_profiles[2];
        std::atomic<uint8_t> m_writeIndex{0};
        uint8_t m_readIndex{0};

        // Temporal sliding context window (30 frames = 300ms) for human syllable & speech formant recognition
        static constexpr int CONTEXT_FRAMES = 30;
        std::vector<float> m_erbHistory;
        std::vector<float> m_realHistory;
        std::vector<float> m_imagHistory;

        // Zero-allocation scratch vectors for ONNX outputs
        std::vector<float> m_output0;
        std::vector<float> m_output1;
    };

}
