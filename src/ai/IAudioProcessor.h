#pragma once
#include "../audio/interfaces/IAudioStage.h"

namespace VoiceClear::AI {

    /**
     * @brief Abstract interface for any AI model processor.
     * Hides all ONNX/Tensor specifics from the real-time AudioPipeline.
     */
    class IAudioProcessor : public Audio::IAudioStage {
    public:
        virtual ~IAudioProcessor() = default;

        /**
         * @brief Resets all recurrent states (e.g. GRU/LSTM hidden states).
         * Must be called during device transitions or underruns.
         */
        virtual void ResetState() noexcept = 0;

        // Note: The inherited Process(AudioBuffer&) acts as ProcessChunk().
    };

}
