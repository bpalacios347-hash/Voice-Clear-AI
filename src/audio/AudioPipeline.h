#pragma once
#include <vector>
#include <memory>
#include <mutex>
#include <thread>
#include <atomic>
#include "interfaces/IAudioStage.h"
#include "../diagnostics/TelemetryManager.h"
#include "LockFreeRingBuffer.h"

namespace VoiceClear::Audio {

    /**
     * @brief Central orchestrator for all Digital Signal Processing hooks.
     * Operates a dedicated lock-free Processing Thread that pops from an 
     * input ring buffer, processes through stages and AI, and pushes to an output buffer.
     */
    class AudioPipeline {
    public:
        AudioPipeline(std::shared_ptr<Diagnostics::TelemetryManager> telemetry,
                      std::shared_ptr<LockFreeRingBuffer<float>> inputBuffer,
                      std::shared_ptr<LockFreeRingBuffer<float>> outputBuffer);
        ~AudioPipeline();

        /**
         * @brief Starts the dedicated Processing Thread.
         */
        bool Start();

        /**
         * @brief Stops the Processing Thread gracefully.
         */
        void Stop();

        /**
         * @brief Injects the synchronous AI InferenceEngine into the DSP chain.
         */
        void SetAIProcessor(std::shared_ptr<IAudioStage> aiEngine);

        /**
         * @brief Enables or disables AI processing dynamically.
         */
        void EnableAI(bool enable) noexcept;

        void AddStage(std::unique_ptr<IAudioStage> stage);
        void ClearStages();

    private:
        std::shared_ptr<Diagnostics::TelemetryManager> m_telemetry;
        std::shared_ptr<LockFreeRingBuffer<float>> m_inputBuffer;
        std::shared_ptr<LockFreeRingBuffer<float>> m_outputBuffer;

        std::vector<std::unique_ptr<IAudioStage>> m_stages;
        std::shared_ptr<IAudioStage> m_aiEngine;
        
        std::atomic<bool> m_aiEnabled{true};
        std::atomic<bool> m_running{false};
        std::thread m_processingThread;

        void ProcessingThreadLoop();
        void ProcessBlock(AudioBuffer& buffer) noexcept;
    };

}
