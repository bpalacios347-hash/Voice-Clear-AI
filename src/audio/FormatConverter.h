#pragma once
#include <cstdint>
#include <vector>
#include <memory>
#include <span>
#include "interfaces/IAudioStage.h"
#include "../diagnostics/TelemetryManager.h"

namespace VoiceClear::Audio {

    enum class AudioSampleFormat {
        Int16,
        Int24,
        Int32,
        Float32,
        Unsupported
    };

    /**
     * @brief Converts incoming audio (various PCM/Float formats, Mono/Stereo) 
     * into the canonical internal processing format: Mono 32-bit Float.
     * Note: Sample rate conversion is handled separately by the SampleRateConverter.
     */
    class FormatConverter {
    public:
        explicit FormatConverter(std::shared_ptr<Diagnostics::TelemetryManager> telemetry);
        ~FormatConverter() = default;

        bool Initialize(AudioSampleFormat inputFormat, int inputChannels);

        /**
         * @brief Converts raw interleaved byte data into canonical Mono Float32.
         * @param inputData Raw byte array from WASAPI capture.
         * @param inputFrames Number of frames in the input.
         * @param outputBuffer Pre-allocated AudioBuffer to store the result.
         * @return True if conversion succeeded without allocation.
         */
        bool Convert(const uint8_t* inputData, size_t inputFrames, AudioBuffer& outputBuffer) noexcept;

    private:
        std::shared_ptr<Diagnostics::TelemetryManager> m_telemetry;
        AudioSampleFormat m_inputFormat{AudioSampleFormat::Unsupported};
        int m_inputChannels{0};

        // Conversion routines
        void ConvertInt16(const uint8_t* input, size_t frames, std::span<float> output) const noexcept;
        void ConvertInt24(const uint8_t* input, size_t frames, std::span<float> output) const noexcept;
        void ConvertInt32(const uint8_t* input, size_t frames, std::span<float> output) const noexcept;
        void ConvertFloat32(const uint8_t* input, size_t frames, std::span<float> output) const noexcept;
    };

}
