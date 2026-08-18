#pragma once
#include <cstdint>

namespace VoiceClear::Audio {

    /**
     * @brief Represents the high-level state of the Audio Engine Pipeline.
     */
    enum class AudioEngineState : uint8_t {
        Initializing,
        Starting,
        Running,
        Recovering,
        Stopping,
        Stopped,
        Error
    };

    /**
     * @brief Configuration for the Adaptive Buffer.
     */
    struct AdaptiveBufferConfig {
        size_t currentFrames{256}; // Can be 128, 256, or 512
        bool autoAdapt{true};
    };

}
