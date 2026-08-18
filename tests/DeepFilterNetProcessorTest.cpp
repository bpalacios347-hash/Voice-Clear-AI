#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include "../src/ai/ModelManager.h"
#include "../src/ai/DeepFilterNetProcessor.h"
#include <vector>
#include <filesystem>
#include <fstream>

using namespace VoiceClear::AI;
using namespace VoiceClear::Audio;

TEST_CASE("DeepFilterNetProcessor Initialization and Inference", "[DeepFilterNet]") {
    ModelManager manager(nullptr);

    // To test without a real model present, we can just ensure the architecture doesn't crash 
    // when a model is missing.
    auto session = manager.LoadModel("deepfilternet3.onnx");
    
    if (!session) {
        // Model not available on host system yet. We will test graceful degradation.
        DeepFilterNetProcessor processor(nullptr);
        REQUIRE(!processor.Initialize(std::move(session)));
    } else {
        DeepFilterNetProcessor processor(nullptr);
        REQUIRE(processor.Initialize(std::move(session)));

        std::vector<float> data(480, 0.5f); // 10ms chunk
        AudioBuffer buffer;
        buffer.samples = data;
        buffer.frames = 480;
        buffer.channels = 1;
        buffer.sampleRate = 48000;
        buffer.metadata.containsSpeech = true;

        REQUIRE_NOTHROW(processor.Process(buffer));
        
        // Reset state
        REQUIRE_NOTHROW(processor.ResetState());
        
        // Ensure no crash on second run
        REQUIRE_NOTHROW(processor.Process(buffer));
    }
}
