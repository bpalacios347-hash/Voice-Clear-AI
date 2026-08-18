#include "DeepFilterNetProcessor.h"
#include "../utils/Logger.h"
#include "../profiling/ProfilingMacros.h"
#include <chrono>
#include <iostream>

namespace VoiceClear::AI {

    DeepFilterNetProcessor::DeepFilterNetProcessor(std::shared_ptr<Diagnostics::TelemetryManager> telemetry)
        : m_telemetry(std::move(telemetry)) {
        
        auto balanced = NoiseProfile::Balanced();
        m_profiles[0].suppressionDepthDb = balanced.suppressionDepthDb;
        m_profiles[0].oversubtraction   = balanced.oversubtraction;
        m_profiles[0].voiceBoostDb      = balanced.voiceBoostDb;
        m_profiles[0].bypass            = balanced.bypass;

        m_profiles[1] = m_profiles[0];
        m_writeIndex.store(0, std::memory_order_relaxed);
        m_readIndex = 0;
    }

    bool DeepFilterNetProcessor::Initialize(std::unique_ptr<Ort::Session> session) {
        if (!session) return false;

        m_session = std::move(session);
        m_context = std::make_unique<InferenceContext>();
        
        try {
            // Probe model dynamically and pre-allocate all tensor buffers
            m_context->Allocate(*m_session);
            // 48kHz, 960 FFT, 32 ERB bands, 96 df_bins
            m_stft = std::make_unique<DSP::STFT>(480, 960);
            m_erb  = std::make_unique<DSP::ErbFeatures>(48000, 960, 32, 96);
            m_erbHistory.assign(CONTEXT_FRAMES * 32, 0.0f);
            m_realHistory.assign(CONTEXT_FRAMES * 96, 0.0f);
            m_imagHistory.assign(CONTEXT_FRAMES * 96, 0.0f);
            Utils::Logger::GetInstance()->Log(Core::LogLevel::Info, "DeepFilterNetProcessor Initialized with STFT/ERB Adaptive DSP.");
            return true;
        } catch (const std::exception& e) {
            Utils::Logger::GetInstance()->Log(Core::LogLevel::Error, std::string("Context allocation failed: ") + e.what());
            return false;
        }
    }

    void DeepFilterNetProcessor::ResetState() noexcept {
        if (m_context) {
            m_context->ResetStates();
        }
        if (m_erb) {
            m_erb->ResetState();
        }
        std::fill(m_erbHistory.begin(), m_erbHistory.end(), 0.0f);
        std::fill(m_realHistory.begin(), m_realHistory.end(), 0.0f);
        std::fill(m_imagHistory.begin(), m_imagHistory.end(), 0.0f);
    }

    void DeepFilterNetProcessor::SetProfile(const NoiseProfile& profile) noexcept {
        uint8_t nextWrite = (m_writeIndex.load(std::memory_order_relaxed) + 1) % 2;
        m_profiles[nextWrite].suppressionDepthDb = profile.suppressionDepthDb;
        m_profiles[nextWrite].oversubtraction   = profile.oversubtraction;
        m_profiles[nextWrite].voiceBoostDb      = profile.voiceBoostDb;
        m_profiles[nextWrite].bypass            = profile.bypass;
        m_writeIndex.store(nextWrite, std::memory_order_release);
    }

    void DeepFilterNetProcessor::Process(Audio::AudioBuffer& buffer) noexcept {
        VC_PROFILE_ZONE("DeepFilterNet_Inference");

        // Lock-free double buffer read: snapshot the latest profile parameters
        uint8_t currentWrite = m_writeIndex.load(std::memory_order_acquire);
        DSP::DspProfileSettings currentProfile = m_profiles[currentWrite];

        if (currentProfile.bypass) {
            return; // Passthrough unaltered
        }

        if (!m_stft || !m_erb) {
            // Lazy initialization of fallback DSP if session was not loaded
            if (!m_stft) m_stft = std::make_unique<DSP::STFT>(480, 960);
            if (!m_erb)  m_erb  = std::make_unique<DSP::ErbFeatures>(48000, 960, 32, 96);
        }

        auto start = std::chrono::high_resolution_clock::now();

        try {
            // 1. Forward STFT (Analysis)
            auto spectrum = m_stft->Forward(buffer.samples);

            // 2. Extract Acoustic Features
            auto featErb  = m_erb->ComputeErbFeatures(spectrum);
            auto featSpec = m_erb->ComputeSpecFeatures(spectrum);

            std::vector<float> output0;
            std::vector<float> output1;

            // 3. Execute ONNX Inference (if session loaded)
            if (m_session && m_context) {
                if (m_erbHistory.size() != CONTEXT_FRAMES * 32) {
                    m_erbHistory.assign(CONTEXT_FRAMES * 32, 0.0f);
                    m_realHistory.assign(CONTEXT_FRAMES * 96, 0.0f);
                    m_imagHistory.assign(CONTEXT_FRAMES * 96, 0.0f);
                }

                // Push new frame into sliding temporal context windows
                // 1. ERB features [1, 1, 5, 32]
                std::copy(m_erbHistory.begin() + 32, m_erbHistory.end(), m_erbHistory.begin());
                std::copy(featErb.begin(), featErb.end(), m_erbHistory.end() - 32);

                // 2. Complex spectrum features [1, 2, 5, 96]
                // featSpec layout from ComputeSpecFeatures: 0..95 Real, 96..191 Imag
                std::copy(m_realHistory.begin() + 96, m_realHistory.end(), m_realHistory.begin());
                std::copy(featSpec.begin(), featSpec.begin() + 96, m_realHistory.end() - 96);

                std::copy(m_imagHistory.begin() + 96, m_imagHistory.end(), m_imagHistory.begin());
                std::copy(featSpec.begin() + 96, featSpec.end(), m_imagHistory.end() - 96);

                // 3. Populate ONNX input buffers with exact tensor layout:
                // Input 0: feat_erb [1, 1, 5, 32] (160 floats)
                auto& input0 = m_context->GetInputBuffer(0);
                std::copy(m_erbHistory.begin(), m_erbHistory.end(), input0.begin());

                // Input 1: feat_spec [1, 2, 5, 96] (960 floats: 480 Real followed by 480 Imag)
                auto& input1 = m_context->GetInputBuffer(1);
                std::copy(m_realHistory.begin(), m_realHistory.end(), input1.begin());
                std::copy(m_imagHistory.begin(), m_imagHistory.end(), input1.begin() + (CONTEXT_FRAMES * 96));

                std::vector<Ort::Value> inputValues = m_context->GetInputValues();

                Ort::RunOptions runOptions;
                auto outputTensors = m_session->Run(
                    runOptions,
                    m_context->GetInputNames().data(),
                    inputValues.data(),
                    inputValues.size(),
                    m_context->GetOutputNames().data(),
                    m_context->GetOutputCount()
                );

                if (!outputTensors.empty()) {
                    const float* out0_ptr = outputTensors[0].GetTensorMutableData<float>();
                    auto shape0 = outputTensors[0].GetTensorTypeAndShapeInfo().GetShape();
                    size_t out0_elements = outputTensors[0].GetTensorTypeAndShapeInfo().GetElementCount();
                    output0.assign(out0_ptr, out0_ptr + out0_elements);

                    if (outputTensors.size() > 1) {
                        const float* out1_ptr = outputTensors[1].GetTensorMutableData<float>();
                        auto shape1 = outputTensors[1].GetTensorTypeAndShapeInfo().GetShape();
                        size_t out1_elements = outputTensors[1].GetTensorTypeAndShapeInfo().GetElementCount();
                        output1.assign(out1_ptr, out1_ptr + out1_elements);
                    }
                    
                    // Propagate RNN states from outputs back to inputs for the next frame
                    // Inputs: 0=erb, 1=spec, 2=state0, 3=state1, ...
                    // Outputs: 0=erb_mask, 1=df_coefs, 2=lsnr, 3=state0, 4=state1, ...
                    size_t outCount = outputTensors.size();
                    size_t inCount = m_context->GetInputCount();
                    
                    for (size_t outIdx = 3; outIdx < outCount; ++outIdx) {
                        size_t inIdx = outIdx - 1; // Map output 3 -> input 2
                        if (inIdx < inCount) {
                            auto& inputBuffer = m_context->GetInputBuffer(inIdx);
                            const float* outStatePtr = outputTensors[outIdx].GetTensorData<float>();
                            size_t elements = outputTensors[outIdx].GetTensorTypeAndShapeInfo().GetElementCount();
                            if (elements == inputBuffer.size()) {
                                std::copy(outStatePtr, outStatePtr + elements, inputBuffer.begin());
                            }
                        }
                    }
                }
            }

            // 4. Apply Adaptive Spectral Wiener Filter + Neural Mask with Profile Settings
            auto filteredSpectrum = m_erb->ApplyFilters(output0, output1, spectrum, currentProfile);

            // 5. Inverse STFT (Synthesis & Overlap-Add)
            auto outputAudio = m_stft->Inverse(filteredSpectrum);

            // 6. Map clean enhanced audio with transparent Soft-Knee Limiter (prevents digital clipping)
            size_t processLen = std::min(buffer.samples.size(), outputAudio.size());
            for (size_t i = 0; i < processLen; ++i) {
                float s = outputAudio[i];
                if (s > 0.92f) {
                    s = 0.92f + 0.08f * std::tanh((s - 0.92f) / 0.08f);
                } else if (s < -0.92f) {
                    s = -0.92f + 0.08f * std::tanh((s + 0.92f) / 0.08f);
                }
                buffer.samples[i] = s;
            }

            auto end = std::chrono::high_resolution_clock::now();
            float durationMs = std::chrono::duration<float, std::milli>(end - start).count();

            if (m_telemetry) {
                m_telemetry->PublishCpuUsage(durationMs);
            }

        } catch (const std::exception& e) {
            Utils::Logger::GetInstance()->Log(Core::LogLevel::Error,
                std::string("Processing exception: ") + e.what());
        } catch (...) {
            Utils::Logger::GetInstance()->Log(Core::LogLevel::Error,
                "Unknown processing exception — audio passed dry.");
        }
    }

}
