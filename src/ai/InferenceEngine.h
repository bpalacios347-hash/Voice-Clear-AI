#pragma once
#include "IAudioProcessor.h"
#include "../profiling/AIProfiler.h"
#include <memory>
#include <chrono>

namespace VoiceClear::AI {

    /**
     * @brief Abstraction over the underlying active AI model.
     * AudioPipeline communicates ONLY with this class.
     * Manages lifecycle, inference execution, and delegates profiling automatically.
     */
    class InferenceEngine : public Audio::IAudioStage {
    public:
        explicit InferenceEngine(std::unique_ptr<IAudioProcessor> processor, std::shared_ptr<Profiling::AIProfiler> profiler = nullptr);
        ~InferenceEngine() override = default;

        /**
         * @brief Injects or hot-swaps the underlying AI processor (e.g. DeepFilterNet).
         * Not real-time safe to call during Process().
         */
        void SetActiveProcessor(std::unique_ptr<IAudioProcessor> processor);

        /**
         * @brief Resets the active processor's recurrent states explicitly.
         */
        void ResetState() noexcept;

        /**
         * @brief Standard DSP hook for the AudioPipeline.
         * Automatically profiles inference latency.
         */
        void Process(Audio::AudioBuffer& buffer) noexcept override;

    private:
        std::shared_ptr<Profiling::AIProfiler> m_profiler;
        std::unique_ptr<IAudioProcessor> m_processor;
    };

}
