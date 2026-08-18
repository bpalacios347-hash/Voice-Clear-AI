#pragma once
#include "interfaces/IAudioStage.h"

namespace VoiceClear::Audio::Stages {

    /**
     * @brief Dedicated bit-depth and layout converter (e.g. 16-bit to 32-bit Float, Mono/Stereo).
     * Does NOT perform resampling.
     */
    class FormatConverter : public IAudioStage {
    public:
        FormatConverter() = default;
        ~FormatConverter() override = default;

        void Process(AudioBlock& block) noexcept override {
            // Placeholder: Fast bit-depth conversion using SIMD
        }
    };

    /**
     * @brief Dedicated resampler (e.g., 44.1kHz to 48kHz).
     */
    class SampleRateConverter : public IAudioStage {
    public:
        SampleRateConverter() = default;
        ~SampleRateConverter() override = default;

        void Process(AudioBlock& block) noexcept override {
            // Placeholder: Fast resampling
        }
    };

}
