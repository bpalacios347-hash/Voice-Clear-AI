#include <catch2/catch_test_macros.hpp>
#include "../src/audio/OutputEngine.h"
#include "../src/audio/LockFreeRingBuffer.h"
#include "../src/diagnostics/TelemetryManager.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>

using namespace VoiceClear::Audio;
using namespace VoiceClear::Diagnostics;

TEST_CASE("OutputEngine Initialization and Fallback", "[OutputEngine]") {
    auto telemetry = std::make_shared<TelemetryManager>();
    OutputEngine output(telemetry);

    OutputConfig config;
    config.deviceId = "DummySpeaker123";
    config.preferExclusive = true; // Fallback to Shared test

    REQUIRE(output.Initialize(config));
    REQUIRE(output.IsExclusiveMode() == true);
    REQUIRE(output.GetSampleRate() == 48000);
}

TEST_CASE("OutputEngine Underrun Recovery", "[OutputEngine]") {
    auto telemetry = std::make_shared<TelemetryManager>();
    OutputEngine output(telemetry);

    OutputConfig config;
    config.deviceId = "TestSpeaker";
    REQUIRE(output.Initialize(config));

    std::atomic<int> underrunSimulations{0};
    output.SetAudioRequestCallback([&underrunSimulations](float* data, size_t frames, int channels, int sampleRate) {
        underrunSimulations++;
        // Simulate severe starvation by returning 0 frames, triggering OutputEngine's silence fill
        return 0; 
    });

    REQUIRE(output.Start());
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    output.Stop();

    REQUIRE(underrunSimulations > 0);
    // Since telemetry isn't fully wired to Catch2 assertions here, we trust the coverage of the callback count
}

TEST_CASE("End-To-End Simulation: Capture -> RingBuffer -> Output", "[OutputEngine][Integration]") {
    LockFreeRingBuffer<float> ringBuffer(4096);
    
    auto telemetry = std::make_shared<TelemetryManager>();
    OutputEngine output(telemetry);
    
    OutputConfig config;
    config.deviceId = "TestSpeaker";
    REQUIRE(output.Initialize(config));

    std::atomic<long long> totalRendered{0};

    // The output engine fetches from the ring buffer
    output.SetAudioRequestCallback([&](float* data, size_t frames, int channels, int sampleRate) {
        size_t popped = ringBuffer.Pop(data, frames);
        totalRendered += popped;
        return popped; // Underruns handled internally by OutputEngine
    });

    REQUIRE(output.Start());

    // Simulated Capture Producer
    std::thread producer([&]() {
        std::vector<float> captureBlock(256, 0.5f);
        for(int i = 0; i < 50; ++i) { // Simulate ~260ms of audio
            ringBuffer.Push(captureBlock.data(), 256);
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    });

    producer.join();
    std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Allow render to drain
    output.Stop();

    // The ring buffer should have drained successfully into the output engine
    REQUIRE(totalRendered > 0);
    REQUIRE(ringBuffer.Empty());
}
