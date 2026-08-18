#include <catch2/catch_test_macros.hpp>
#include "../src/driver/DriverBridge.h"
#include <vector>
#include <thread>

using namespace VoiceClear::Driver;
using namespace VoiceClear::Audio;

TEST_CASE("DriverBridge Mock Connection and Streaming", "[DriverBridge]") {
    DriverBridge bridge(nullptr);

    // Should successfully create mock shared memory even if driver is missing
    REQUIRE(bridge.Connect());
    REQUIRE(bridge.IsConnected());

    std::vector<float> data(256, 0.5f);
    AudioBuffer buffer;
    buffer.samples = data;
    buffer.frames = 256;
    buffer.channels = 1;
    buffer.sampleRate = 48000;

    // Send audio
    REQUIRE(bridge.SendAudio(buffer));

    // Simulate disconnect and recovery
    bridge.Disconnect();
    REQUIRE(!bridge.IsConnected());
    REQUIRE(!bridge.SendAudio(buffer)); // Should gracefully drop

    REQUIRE(bridge.Connect());
    REQUIRE(bridge.IsConnected());
    REQUIRE(bridge.SendAudio(buffer));
}
