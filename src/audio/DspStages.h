#pragma once
#include "interfaces/IAudioStage.h"

namespace VoiceClear::Audio::Stages {

    class InputGainStage : public IAudioStage {
    public:
        explicit InputGainStage(float gainLinear) : m_gain(gainLinear) {}
        void Process(AudioBuffer& buffer) noexcept override {
            for (uint32_t i = 0; i < buffer.frames; ++i) {
                buffer.samples[i] *= m_gain;
            }
        }
    private:
        float m_gain{1.0f};
    };

    class HighPassFilterStage : public IAudioStage {
    public:
        void Process(AudioBuffer& buffer) noexcept override {
            // Simple first-order IIR high-pass filter (approx 80Hz cutoff at 48kHz)
            constexpr float alpha = 0.98f; 
            for (uint32_t i = 0; i < buffer.frames; ++i) {
                float x = buffer.samples[i];
                float y = alpha * (m_prevY + x - m_prevX);
                m_prevX = x;
                m_prevY = y;
                buffer.samples[i] = y;
            }
        }
    private:
        float m_prevX{0.0f};
        float m_prevY{0.0f};
    };

    class DcOffsetRemovalStage : public IAudioStage {
    public:
        void Process(AudioBuffer& buffer) noexcept override {
            // Leaky integrator to estimate and remove DC offset
            constexpr float alpha = 0.999f;
            for (uint32_t i = 0; i < buffer.frames; ++i) {
                m_dcEstimate = (alpha * m_dcEstimate) + ((1.0f - alpha) * buffer.samples[i]);
                buffer.samples[i] -= m_dcEstimate;
            }
        }
    private:
        float m_dcEstimate{0.0f};
    };

    class VadStage : public IAudioStage {
    public:
        void Process(AudioBuffer& buffer) noexcept override {
            // Basic energy thresholding for VAD scaffolding
            float energy = 0.0f;
            for (uint32_t i = 0; i < buffer.frames; ++i) {
                energy += buffer.samples[i] * buffer.samples[i];
            }
            buffer.metadata.containsSpeech = (energy > 0.0001f);
        }
    };

    class LimiterStage : public IAudioStage {
    public:
        void Process(AudioBuffer& buffer) noexcept override {
            // Simple hard clip scaffolding (A real lookahead limiter goes here later)
            for (uint32_t i = 0; i < buffer.frames; ++i) {
                if (buffer.samples[i] > 1.0f) buffer.samples[i] = 1.0f;
                else if (buffer.samples[i] < -1.0f) buffer.samples[i] = -1.0f;
            }
        }
    };

}
