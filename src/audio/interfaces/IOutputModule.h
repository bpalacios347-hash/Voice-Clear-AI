#pragma once

#include <span>

namespace VoiceClear::Audio {

    /**
     * @brief Interface for outputting audio to the Virtual Microphone Driver.
     */
    class IOutputModule {
    public:
        virtual ~IOutputModule() = default;

        /**
         * @brief Initialize output to the driver.
         * @param sampleRate The output sample rate.
         * @param channels Number of channels.
         * @return true if successful.
         */
        virtual bool Initialize(int sampleRate, int channels) = 0;

        /**
         * @brief Push processed audio to the driver.
         * Must be non-blocking and zero-allocation (e.g., via shared memory or lock-free ring buffer).
         * @param buffer The processed audio samples.
         */
        virtual void PushAudio(std::span<const float> buffer) = 0;

        virtual bool Start() = 0;
        virtual void Stop() = 0;
    };

} // namespace VoiceClear::Audio
