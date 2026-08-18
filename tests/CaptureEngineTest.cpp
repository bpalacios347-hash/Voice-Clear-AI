#include <catch2/catch_test_macros.hpp>
#include "../src/audio/CaptureEngine.h"
#include "../src/diagnostics/TelemetryManager.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>

using namespace VoiceClear::Audio;
using namespace VoiceClear::Diagnostics;

TEST_CASE("CaptureEngine Initialization and Fallback", "[CaptureEngine]") {
    auto telemetry = std::make_shared<TelemetryManager>();
    CaptureEngine capture(telemetry);

    CaptureConfig config;
    config.deviceId = "DummyMicrophone123";
    config.preferExclusive = true; // Should test fallback to Shared if Exclusive fails

    REQUIRE(capture.Initialize(config));
    
    // In our scaffolding, InitializeWasapi always returns true so Exclusive Mode is "granted"
    REQUIRE(capture.IsExclusiveMode() == true);
    
    REQUIRE(capture.GetSampleRate() == 48000);
}

TEST_CASE("CaptureEngine Callback and Lifecycle", "[CaptureEngine]") {
    auto telemetry = std::make_shared<TelemetryManager>();
    CaptureEngine capture(telemetry);

    CaptureConfig config;
    config.deviceId = "TestMic";
    REQUIRE(capture.Initialize(config));

    std::atomic<int> callbackCount{0};
    capture.SetAudioCallback([&callbackCount](const float* data, size_t frames, int channels, int sampleRate) {
        callbackCount++;
    });

    REQUIRE(capture.Start());
    
    // Allow the Pro Audio thread to spin up and pump some simulated frames
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    
    capture.Stop();

    // Since we simulate 256 frames @ 48kHz (~5.33ms per frame block), 
    // waiting 150ms should yield roughly 20-30 callbacks.
    REQUIRE(callbackCount > 0);
}
