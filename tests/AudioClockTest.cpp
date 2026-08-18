#include <catch2/catch_test_macros.hpp>
#include "../src/audio/AudioClock.h"
#include <thread>
#include <chrono>

using namespace VoiceClear::Audio;

TEST_CASE("AudioClock Timestamp Accuracy", "[AudioClock]") {
    uint64_t t1 = AudioClock::Now();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    uint64_t t2 = AudioClock::Now();
    
    REQUIRE(t2 > t1);
    double elapsedMs = AudioClock::TicksToMs(t2 - t1);
    REQUIRE(elapsedMs >= 4.0); // Allow OS scheduling variance
}

TEST_CASE("AudioClock E2E Latency and Jitter", "[AudioClock]") {
    AudioClock clock(nullptr); // No telemetry
    
    AudioBuffer buffer;
    buffer.sampleRate = 48000;
    buffer.frames = 256; // 5.33ms expected interval
    
    // Simulate first block
    clock.RecordCaptureTimestamp(buffer);
    std::this_thread::sleep_for(std::chrono::milliseconds(8)); // Simulate 8ms pipeline latency
    clock.RecordRenderTimestamp(buffer);
    clock.AnalyzeBlock(buffer);

    auto stats = clock.GetStatistics();
    REQUIRE(stats.averageLatencyMs >= 7.5);
    REQUIRE(stats.peakLatencyMs >= 7.5);
    REQUIRE(stats.maxJitterMs == 0.0); // No jitter on first block

    // Simulate second block with exactly 5.33ms delta
    uint64_t currentRender = buffer.metadata.renderTimestamp;
    
    AudioBuffer buffer2;
    buffer2.sampleRate = 48000;
    buffer2.frames = 256;
    buffer2.metadata.captureTimestamp = buffer.metadata.captureTimestamp + (currentRender - buffer.metadata.captureTimestamp); // Shift
    
    // We'll just fake exact ticks to test logic precisely
    uint64_t expectedIntervalTicks = static_cast<uint64_t>(5.333333 / AudioClock::TicksToMs(1));
    buffer2.metadata.renderTimestamp = currentRender + expectedIntervalTicks;
    buffer2.metadata.captureTimestamp = buffer.metadata.captureTimestamp + expectedIntervalTicks;
    
    clock.AnalyzeBlock(buffer2);
    
    stats = clock.GetStatistics();
    // Jitter should be extremely low because we faked a perfect interval
    REQUIRE(stats.maxJitterMs < 0.5);
}
