#include "InferenceContext.h"
#include "../utils/Logger.h"
#include <onnxruntime_cxx_api.h>
#include <stdexcept>
#include <iostream>

namespace VoiceClear::AI {

    InferenceContext::InferenceContext() {
        m_memoryInfo = std::make_unique<Ort::MemoryInfo>(Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault));
    }

    InferenceContext::~InferenceContext() = default;

    void InferenceContext::Allocate(Ort::Session& session) {
        Ort::AllocatorWithDefaultOptions allocator;

        size_t numInputNodes = session.GetInputCount();
        size_t numOutputNodes = session.GetOutputCount();

        Utils::Logger::GetInstance()->Log(Core::LogLevel::Info, "ONNX Model Topology:");
        Utils::Logger::GetInstance()->Log(Core::LogLevel::Info, "- Input Count: " + std::to_string(numInputNodes));
        Utils::Logger::GetInstance()->Log(Core::LogLevel::Info, "- Output Count: " + std::to_string(numOutputNodes));

        if (numInputNodes == 0 || numOutputNodes == 0) {
            throw std::runtime_error("Invalid ONNX Model: Zero input or output nodes.");
        }

        // Dynamically discover inputs
        for (size_t i = 0; i < numInputNodes; i++) {
            Ort::AllocatedStringPtr inputNamePtr = session.GetInputNameAllocated(i, allocator);
            m_inputNamesStrings.push_back(inputNamePtr.get());
            
            Ort::TypeInfo typeInfo = session.GetInputTypeInfo(i);
            auto tensorInfo = typeInfo.GetTensorTypeAndShapeInfo();
            std::vector<int64_t> shape = tensorInfo.GetShape();

            std::string shapeStr = "[";
            for (size_t j = 0; j < shape.size(); j++) {
                shapeStr += std::to_string(shape[j]) + (j < shape.size() - 1 ? ", " : "");
            }
            shapeStr += "]";
            std::cout << "- Input " << i << ": " << inputNamePtr.get() << " Shape: " << shapeStr << std::endl;
            Utils::Logger::GetInstance()->Log(Core::LogLevel::Info, "- Input " + std::to_string(i) + ": " + inputNamePtr.get() + " Shape: " + shapeStr);

            for (size_t j = 0; j < shape.size(); ++j) {
                if (shape[j] <= 0) {
                    if (j == 0) {
                        shape[j] = 1; // Batch dimension
                    } else {
                        shape[j] = 30; // 30-frame temporal context (300ms) for human speech recognition
                    }
                }
            }

            size_t totalElements = 1;
            for (auto dim : shape) totalElements *= dim;

            m_inputShapes.push_back(shape);
            m_inputBuffers.emplace_back(totalElements, 0.0f);
        }

        for (size_t i = 0; i < numInputNodes; i++) {
            m_inputNames.push_back(m_inputNamesStrings[i].c_str());
        }

        // Dynamically discover outputs
        for (size_t i = 0; i < numOutputNodes; i++) {
            Ort::AllocatedStringPtr outputNamePtr = session.GetOutputNameAllocated(i, allocator);
            m_outputNamesStrings.push_back(outputNamePtr.get());
            
            Ort::TypeInfo typeInfo = session.GetOutputTypeInfo(i);
            auto tensorInfo = typeInfo.GetTensorTypeAndShapeInfo();
            std::vector<int64_t> shape = tensorInfo.GetShape();
            std::string shapeStr = "[";
            for (size_t j = 0; j < shape.size(); j++) {
                shapeStr += std::to_string(shape[j]) + (j < shape.size() - 1 ? ", " : "");
            }
            shapeStr += "]";
            std::cout << "- Output " << i << ": " << outputNamePtr.get() << " Shape: " << shapeStr << std::endl;
        }

        for (size_t i = 0; i < numOutputNodes; i++) {
            m_outputNames.push_back(m_outputNamesStrings[i].c_str());
        }
    }

    std::vector<Ort::Value> InferenceContext::GetInputValues() {
        std::vector<Ort::Value> values;
        values.reserve(m_inputBuffers.size());
        for (size_t i = 0; i < m_inputBuffers.size(); ++i) {
            values.push_back(Ort::Value::CreateTensor<float>(
                *m_memoryInfo,
                m_inputBuffers[i].data(),
                m_inputBuffers[i].size(),
                m_inputShapes[i].data(),
                m_inputShapes[i].size()
            ));
        }
        return values;
    }

    void InferenceContext::ResetStates() noexcept {
        for (auto& buffer : m_inputBuffers) {
            std::fill(buffer.begin(), buffer.end(), 0.0f);
        }
    }
}
