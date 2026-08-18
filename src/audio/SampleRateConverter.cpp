#include "SampleRateConverter.h"
#include "../profiling/ProfilingMacros.h"
#include "../utils/Logger.h"
#include <algorithm>
#include <cmath>

namespace VoiceClear::Audio {

    SampleRateConverter::SampleRateConverter(std::shared_ptr<Diagnostics::TelemetryManager> telemetry)
        : m_telemetry(std::move(telemetry)) {}

    bool SampleRateConverter::Initialize(int inputSampleRate, int outputSampleRate) {
        if (inputSampleRate <= 0 || outputSampleRate <= 0) {
            Utils::Logger::GetInstance()->Log(Core::LogLevel::Error, "Invalid sample rates provided.");
            return false;
        }

        m_inRate = inputSampleRate;
        m_outRate = outputSampleRate;
        m_ratio = static_cast<double>(m_inRate) / static_cast<double>(m_outRate);
        m_phase = 0.0;
        
        m_history[0] = 0.0f;
        m_history[1] = 0.0f;
        m_history[2] = 0.0f;

        Utils::Logger::GetInstance()->Log(Core::LogLevel::Info, 
            "SampleRateConverter initialized: " + std::to_string(m_inRate) + " -> " + std::to_string(m_outRate));
        return true;
    }

    void SampleRateConverter::Reset() noexcept {
        m_phase = 0.0;
        m_history[0] = 0.0f;
        m_history[1] = 0.0f;
        m_history[2] = 0.0f;
    }

    inline float SampleRateConverter::InterpolateCubic(float x0, float x1, float x2, float x3, float t) noexcept {
        // Optimal 3rd order Hermite polynomial
        float c0 = x1;
        float c1 = 0.5f * (x2 - x0);
        float c2 = x0 - 2.5f * x1 + 2.0f * x2 - 0.5f * x3;
        float c3 = 0.5f * (x3 - x0) + 1.5f * (x1 - x2);
        return ((c3 * t + c2) * t + c1) * t + c0;
    }

    bool SampleRateConverter::Convert(const AudioBuffer& inputBuffer, AudioBuffer& outputBuffer) noexcept {
        VC_PROFILE_ZONE("SampleRateConverter_Convert");

        // Passthrough fast-path
        if (m_inRate == m_outRate) {
            size_t copyFrames = std::min(static_cast<size_t>(inputBuffer.frames), outputBuffer.samples.size());
            std::copy_n(inputBuffer.samples.data(), copyFrames, outputBuffer.samples.data());
            
            outputBuffer.frames = static_cast<uint32_t>(copyFrames);
            outputBuffer.channels = 1;
            outputBuffer.sampleRate = m_outRate;
            outputBuffer.metadata = inputBuffer.metadata;
            return true;
        }

        size_t inLen = inputBuffer.frames;
        const float* inSamples = inputBuffer.samples.data();
        float* outSamples = outputBuffer.samples.data();
        size_t outCapacity = outputBuffer.samples.size();
        
        size_t outIndex = 0;
        
        // Loop over expected output frames
        while (outIndex < outCapacity) {
            size_t inIndex = static_cast<size_t>(m_phase + 1e-9);
            if (inIndex >= inLen) {
                break; // Need more input data
            }

            double frac = m_phase - std::floor(m_phase);
            float t = static_cast<float>(frac);

            // Fetch the 4 points required for cubic interpolation, falling back to history for boundary conditions
            float x0 = (inIndex >= 1) ? inSamples[inIndex - 1] : m_history[2];
            float x1 = inSamples[inIndex];
            float x2 = (inIndex + 1 < inLen) ? inSamples[inIndex + 1] : inSamples[inIndex]; // Clamped at tail for simplicity
            float x3 = (inIndex + 2 < inLen) ? inSamples[inIndex + 2] : x2;

            // Handle the boundary where inIndex == 0 and we need history
            if (inIndex == 0) {
                x0 = m_history[2];
            }

            outSamples[outIndex++] = InterpolateCubic(x0, x1, x2, x3, t);
            m_phase += m_ratio;
        }

        // Update state history for the next block
        if (inLen >= 3) {
            m_history[0] = inSamples[inLen - 3];
            m_history[1] = inSamples[inLen - 2];
            m_history[2] = inSamples[inLen - 1];
        } else if (inLen > 0) {
            m_history[2] = inSamples[inLen - 1];
            if (inLen == 2) m_history[1] = inSamples[inLen - 2];
        }

        // Wrap phase back relative to the consumed input
        m_phase -= static_cast<double>(inLen);
        if (m_phase < -1e-6) m_phase = 0.0; // Failsafe for major drift
        else if (m_phase < 0.0) m_phase = 0.0; // Small negative epsilon clamped to 0

        outputBuffer.frames = static_cast<uint32_t>(outIndex);
        outputBuffer.channels = 1;
        outputBuffer.sampleRate = m_outRate;
        outputBuffer.metadata = inputBuffer.metadata;

        return true;
    }
}
