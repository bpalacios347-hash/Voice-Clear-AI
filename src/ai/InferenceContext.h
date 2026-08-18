#pragma once
#include <vector>
#include <string>
#include <memory>
// Forward declare Ort types to hide them from headers
namespace Ort {
    class Session;
    class Env;
    class MemoryInfo;
    class Value;
}

namespace VoiceClear::AI {

    /**
     * @brief Encapsulates tensor memory, scratch buffers, and recurrent state.
     * Allocates strictly during initialization to ensure zero allocations during inference.
     */
    class InferenceContext {
    public:
        InferenceContext();
        ~InferenceContext();

        // Disallow copy/move to guarantee stable memory addresses for ONNX
        InferenceContext(const InferenceContext&) = delete;
        InferenceContext& operator=(const InferenceContext&) = delete;

        /**
         * @brief Pre-allocates all necessary tensors and memory arenas based on model dims.
         */
        void Allocate(Ort::Session& session);

        /**
         * @brief Resets recurrent state buffers to zero.
         */
        void ResetStates() noexcept;

        // Pointers for internal execution
        std::vector<const char*> GetInputNames() const { return m_inputNames; }
        std::vector<const char*> GetOutputNames() const { return m_outputNames; }
        
        std::vector<Ort::Value> GetInputValues();
        
        size_t GetInputCount() const { return m_inputBuffers.size(); }
        size_t GetOutputCount() const { return m_outputNames.size(); }

        // Specific buffers mapped to tensors
        std::vector<float>& GetInputBuffer(size_t index) { return m_inputBuffers.at(index); }

    private:
        std::unique_ptr<Ort::MemoryInfo> m_memoryInfo;
        
        // Raw buffer storage ensuring lifetime stability
        std::vector<std::vector<float>> m_inputBuffers;
        std::vector<std::vector<int64_t>> m_inputShapes;

        std::vector<std::string> m_inputNamesStrings;
        std::vector<std::string> m_outputNamesStrings;
        std::vector<const char*> m_inputNames;
        std::vector<const char*> m_outputNames;
    };

}
