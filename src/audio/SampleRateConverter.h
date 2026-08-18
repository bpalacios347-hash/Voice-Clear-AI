#pragma once
#include <vector>
#include <memory>
#include <cstdint>
#include "interfaces/IAudioStage.h"
#include "../diagnostics/TelemetryManager.h"

namespace VoiceClear::Audio {

    /**
     * @brief High-performance, zero-allocation Sample Rate Converter.
     * Uses a cubic Hermite polynomial interpolator suitable for real-time DSP.
     * Enforces the internal 48kHz Mono Float32 standard.
     */
    class SampleRateConverter {
    public:
        explicit SampleRateConverter(std::shared_ptr<Diagnostics::TelemetryManager> telemetry);
        ~SampleRateConverter() = default;

        /**
         * @brief Initializes the converter. Pre-allocates any necessary state.
         * @param inputSampleRate The sample rate of the capture device.
         * @param outputSampleRate The target canonical sample rate (e.g. 48000).
         */
        bool Initialize(int inputSampleRate, int outputSampleRate);

        /**
         * @brief Resets the interpolation state history (e.g. on device change or underrun).
         */
        void Reset() noexcept;

        /**
         * @brief Converts the sample rate of the input buffer and writes to the output buffer.
         * @param inputBuffer Canonical input audio (must be Mono Float32).
         * @param outputBuffer Pre-allocated canonical output audio buffer.
         * @return True if successful.
         */
        bool Convert(const AudioBuffer& inputBuffer, AudioBuffer& outputBuffer) noexcept;

        int GetInputSampleRate() const noexcept { return m_inRate; }
        int GetOutputSampleRate() const noexcept { return m_outRate; }

    private:
        std::shared_ptr<Diagnostics::TelemetryManager> m_telemetry;
        
        int m_inRate{48000};
        int m_outRate{48000};
        double m_ratio{1.0}; // inRate / outRate
        double m_phase{0.0};

        // State history for cubic interpolation across buffer boundaries
        float m_history[3]{0.0f, 0.0f, 0.0f};

        // Cubic Hermite Interpolation core
        static inline float InterpolateCubic(float x0, float x1, float x2, float x3, float t) noexcept;
    };

}
