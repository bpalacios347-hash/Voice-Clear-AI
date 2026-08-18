#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include "../src/diagnostics/LockFreeTelemetryQueue.h"
#include "../src/audio/DspStages.h"
#include <vector>

TEST_CASE("Telemetry Queue Performance", "[benchmark][diagnostics]") {
    VoiceClear::Diagnostics::LockFreeTelemetryQueue<int> queue(1024);
    BENCHMARK("Push and Pop") {
        queue.Push(42);
        int val;
        queue.Pop(val);
        return val;
    };
}

TEST_CASE("DSP Stage Performance", "[benchmark][audio]") {
    VoiceClear::Audio::Stages::InputGainStage gainStage(1.0f);
    std::vector<float> dummySamples(256, 0.5f);
    VoiceClear::Audio::AudioBuffer buffer;
    buffer.samples = dummySamples;
    buffer.sampleRate = 48000;
    buffer.channels = 1;
    buffer.frames = 256;
    buffer.metadata.containsSpeech = true;

    BENCHMARK("Input Gain Processing") {
        gainStage.Process(buffer);
        return buffer.samples[0];
    };
}
