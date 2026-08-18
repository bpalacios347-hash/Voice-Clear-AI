#pragma once
#include "interfaces/AudioEngineState.h"
#include "interfaces/IAudioStage.h"
#include "DeviceManager.h"
#include <vector>
#include <memory>
#include <atomic>

namespace VoiceClear::Audio {

    /**
     * @brief Orchestrates the entire DSP pipeline and manages the AudioEngineState.
     */
    class AudioPipelineOrchestrator {
    public:
        AudioPipelineOrchestrator() = default;
        ~AudioPipelineOrchestrator() = default;

        void Initialize();
        void Start();
        void Stop();

        // Used by DeviceManager hot-plug hooks to trigger <2s recovery
        void TriggerRecovery();

        AudioEngineState GetCurrentState() const { return m_state.load(std::memory_order_acquire); }

    private:
        void ProcessLoop(); // Simulates the real-time processing loop

        std::atomic<AudioEngineState> m_state{AudioEngineState::Stopped};
        std::vector<std::unique_ptr<IAudioStage>> m_stages;
    };

}
