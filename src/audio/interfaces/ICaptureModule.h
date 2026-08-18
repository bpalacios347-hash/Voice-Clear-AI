#pragma once
#include <string>

#include <span>
#include <functional>

namespace VoiceClear::Audio {

    /**
     * @brief Interface for capturing audio from the physical microphone.
     * Implementations (e.g., WASAPI) will use this to stream data into the DSP pipeline.
     */
    class ICaptureModule {
    public:
        using AudioCallback = std::function<void(std::span<const float>)>;

        virtual ~ICaptureModule() = default;

        /**
         * @brief Initialize capture device.
         * @param deviceId The ID of the microphone to capture from.
         * @param sampleRate The desired sample rate.
         * @return true if successfully initialized.
         */
        virtual bool Initialize(const std::string& deviceId, int sampleRate) = 0;

        /**
         * @brief Register the callback that will receive audio frames.
         * Must be lock-free on the caller side.
         * @param callback The function to call when data is ready.
         */
        virtual void SetCaptureCallback(AudioCallback callback) = 0;

        virtual bool Start() = 0;
        virtual void Stop() = 0;
    };

} // namespace VoiceClear::Audio
