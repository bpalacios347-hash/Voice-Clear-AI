#include <catch2/catch_test_macros.hpp>
#include "../src/audio/DeviceManager.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>

using namespace VoiceClear::Audio;

TEST_CASE("DeviceManager Lifecycle", "[DeviceManager]") {
    DeviceManager manager;
    
    REQUIRE(manager.Initialize());
    REQUIRE(manager.Start());
    
    // Test multiple rapid start/stop cycles
    manager.Stop();
    manager.Shutdown();
    
    for(int i = 0; i < 5; ++i) {
        REQUIRE(manager.Initialize());
        REQUIRE(manager.Start());
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        manager.Shutdown();
    }
}

TEST_CASE("DeviceManager Enumeration", "[DeviceManager]") {
    DeviceManager manager;
    REQUIRE(manager.Initialize());
    
    auto devices = manager.EnumerateInputDevices();
    // Assuming the test runner machine has at least one audio endpoint.
    // If not, this might be empty.
    std::cout << "Found " << devices.size() << " capture endpoints.\n";
    for (const auto& dev : devices) {
        std::cout << " - " << dev.friendlyName << " [" << dev.id << "]\n";
    }
}

TEST_CASE("DeviceManager Event Callback", "[DeviceManager]") {
    DeviceManager manager;
    REQUIRE(manager.Initialize());
    
    std::atomic<int> eventCount{0};
    manager.RegisterEventCallback([&eventCount](const DeviceEvent& e) {
        eventCount++;
        std::cout << "Received Event for Device: " << e.deviceId << "\n";
    });
    
    REQUIRE(manager.Start());
    
    // We cannot reliably trigger a physical hotplug in CI, 
    // but we can ensure the threads spin up and don't crash.
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    manager.Shutdown();
}
