#include "InferenceEngine.h"

namespace VoiceClear::AI {

    InferenceEngine::InferenceEngine(std::unique_ptr<IAudioProcessor> processor, std::shared_ptr<Profiling::AIProfiler> profiler)
        : m_processor(std::move(processor)), m_profiler(std::move(profiler)) {}

    void InferenceEngine::SetActiveProcessor(std::unique_ptr<IAudioProcessor> processor) {
        m_processor = std::move(processor);
        if (m_profiler) {
            m_profiler->IncrementResetCount();
        }
    }

    void InferenceEngine::ResetState() noexcept {
        if (m_processor) {
            m_processor->ResetState();
            if (m_profiler) {
                m_profiler->IncrementResetCount();
            }
        }
    }

    void InferenceEngine::Process(Audio::AudioBuffer& buffer) noexcept {
        if (!m_processor) return;

        auto start = std::chrono::high_resolution_clock::now();
        
        // Execute the underlying ONNX inference securely encapsulated
        m_processor->Process(buffer);
        
        auto end = std::chrono::high_resolution_clock::now();

        if (m_profiler) {
            float durationMs = std::chrono::duration<float, std::milli>(end - start).count();
            m_profiler->RecordInference(durationMs);
        }
    }

}
