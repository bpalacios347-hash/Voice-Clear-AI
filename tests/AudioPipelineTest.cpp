#include <catch2/catch_test_macros.hpp>
#include "../src/audio/AudioPipeline.h"
#include "../src/audio/DspStages.h"
#include <vector>

using namespace VoiceClear::Audio;

TEST_CASE("AudioPipeline Sequential Execution", "[AudioPipeline]") {
    AudioPipeline pipeline(nullptr);

    pipeline.AddStage(std::make_unique<Stages::InputGainStage>(2.0f));
    pipeline.AddStage(std::make_unique<Stages::DcOffsetRemovalStage>());
    pipeline.AddStage(std::make_unique<Stages::VadStage>());
    pipeline.AddStage(std::make_unique<Stages::LimiterStage>());

    std::vector<float> data = { 0.2f, 0.4f, 0.6f, -0.2f }; // Length 4
    AudioBuffer buffer;
    buffer.samples = data;
    buffer.frames = 4;
    buffer.channels = 1;
    buffer.sampleRate = 48000;
    
    // Process through pipeline
    pipeline.ProcessBlock(buffer);

    // InputGain -> Multiplies by 2.0 -> {0.4, 0.8, 1.2, -0.4}
    // DcOffset -> Slightly modifies them (leaky integrator)
    // Limiter -> Clamps 1.2 to 1.0.

    REQUIRE(buffer.frames == 4);
    REQUIRE(buffer.samples[2] == 1.0f); // Was 1.2, clipped to 1.0 by limiter
    REQUIRE(buffer.metadata.containsSpeech == true); // VAD should detect energy
}
