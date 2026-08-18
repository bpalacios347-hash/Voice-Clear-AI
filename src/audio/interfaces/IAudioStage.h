#pragma once
#include <span>
#include <vector>

namespace VoiceClear::Audio {

    struct AudioBlockMetadata {
        // Timing metadata (in high-resolution microseconds/ticks)
        uint64_t captureTimestamp{0};
        uint64_t ringBufferEntryTimestamp{0};
        uint64_t ringBufferExitTimestamp{0};
        uint64_t renderTimestamp{0};
        
        // Indicates if this block contains valid speech (set by VAD)
        bool containsSpeech{true};
    };

    /**
     * @brief Canonical audio representation that flows through the entire DSP pipeline.
     */
    struct AudioBuffer {
        std::span<float> samples;
        uint32_t frames;
        uint32_t channels;
        uint32_t sampleRate;
        AudioBlockMetadata metadata;
    };

    /**
     * @brief Interface for a single discrete step in the DSP pipeline.
     */
    class IAudioStage {
    public:
        virtual ~IAudioStage() = default;

        /**
         * @brief Process the audio buffer in-place.
         * Must be strictly lock-free, zero-allocation, and real-time safe.
         * @param buffer The audio buffer to process.
         */
        virtual void Process(AudioBuffer& buffer) noexcept = 0;
    };

}
