#include "FormatConverter.h"
#include "../utils/Logger.h"
#include "../profiling/ProfilingMacros.h"

namespace VoiceClear::Audio {

    FormatConverter::FormatConverter(std::shared_ptr<Diagnostics::TelemetryManager> telemetry)
        : m_telemetry(std::move(telemetry)) {}

    bool FormatConverter::Initialize(AudioSampleFormat inputFormat, int inputChannels) {
        if (inputChannels < 1 || inputChannels > 2) {
            Utils::Logger::GetInstance()->Log(Core::LogLevel::Error, "FormatConverter unsupported channels: " + std::to_string(inputChannels));
            return false;
        }
        if (inputFormat == AudioSampleFormat::Unsupported) {
            Utils::Logger::GetInstance()->Log(Core::LogLevel::Error, "FormatConverter unsupported format.");
            return false;
        }
        m_inputFormat = inputFormat;
        m_inputChannels = inputChannels;
        return true;
    }

    bool FormatConverter::Convert(const uint8_t* inputData, size_t inputFrames, AudioBuffer& outputBuffer) noexcept {
        VC_PROFILE_ZONE("FormatConverter_Convert");
        
        if (!inputData || outputBuffer.samples.size() < inputFrames) {
            return false;
        }

        switch (m_inputFormat) {
            case AudioSampleFormat::Int16:
                ConvertInt16(inputData, inputFrames, outputBuffer.samples);
                break;
            case AudioSampleFormat::Int24:
                ConvertInt24(inputData, inputFrames, outputBuffer.samples);
                break;
            case AudioSampleFormat::Int32:
                ConvertInt32(inputData, inputFrames, outputBuffer.samples);
                break;
            case AudioSampleFormat::Float32:
                ConvertFloat32(inputData, inputFrames, outputBuffer.samples);
                break;
            default:
                return false;
        }

        outputBuffer.frames = static_cast<uint32_t>(inputFrames);
        outputBuffer.channels = 1; // Always canonical Mono

        return true;
    }

    void FormatConverter::ConvertInt16(const uint8_t* input, size_t frames, std::span<float> output) const noexcept {
        const int16_t* in16 = reinterpret_cast<const int16_t*>(input);
        constexpr float factor = 1.0f / 32768.0f;
        
        if (m_inputChannels == 1) {
            for (size_t i = 0; i < frames; ++i) {
                output[i] = static_cast<float>(in16[i]) * factor;
            }
        } else if (m_inputChannels == 2) {
            for (size_t i = 0; i < frames; ++i) {
                float l = static_cast<float>(in16[i * 2]) * factor;
                float r = static_cast<float>(in16[i * 2 + 1]) * factor;
                output[i] = (l + r) * 0.5f; // Energy-preserving downmix
            }
        }
    }

    void FormatConverter::ConvertInt24(const uint8_t* input, size_t frames, std::span<float> output) const noexcept {
        constexpr float factor = 1.0f / 8388608.0f;
        
        for (size_t i = 0; i < frames; ++i) {
            float monoSample = 0.0f;
            for (int ch = 0; ch < m_inputChannels; ++ch) {
                size_t offset = (i * m_inputChannels + ch) * 3;
                // Sign-extend 24-bit to 32-bit
                int32_t sample = (input[offset] << 8) | (input[offset + 1] << 16) | (input[offset + 2] << 24);
                sample >>= 8; 
                monoSample += static_cast<float>(sample) * factor;
            }
            output[i] = monoSample / static_cast<float>(m_inputChannels);
        }
    }

    void FormatConverter::ConvertInt32(const uint8_t* input, size_t frames, std::span<float> output) const noexcept {
        const int32_t* in32 = reinterpret_cast<const int32_t*>(input);
        constexpr float factor = 1.0f / 2147483648.0f;
        
        if (m_inputChannels == 1) {
            for (size_t i = 0; i < frames; ++i) {
                output[i] = static_cast<float>(in32[i]) * factor;
            }
        } else if (m_inputChannels == 2) {
            for (size_t i = 0; i < frames; ++i) {
                float l = static_cast<float>(in32[i * 2]) * factor;
                float r = static_cast<float>(in32[i * 2 + 1]) * factor;
                output[i] = (l + r) * 0.5f;
            }
        }
    }

    void FormatConverter::ConvertFloat32(const uint8_t* input, size_t frames, std::span<float> output) const noexcept {
        const float* inF = reinterpret_cast<const float*>(input);
        
        if (m_inputChannels == 1) {
            for (size_t i = 0; i < frames; ++i) {
                output[i] = inF[i];
            }
        } else if (m_inputChannels == 2) {
            for (size_t i = 0; i < frames; ++i) {
                output[i] = (inF[i * 2] + inF[i * 2 + 1]) * 0.5f;
            }
        }
    }
}
