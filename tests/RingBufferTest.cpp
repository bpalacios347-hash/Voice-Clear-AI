#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include "../src/audio/LockFreeRingBuffer.h"
#include <thread>
#include <vector>
#include <atomic>

using namespace VoiceClear::Audio;

TEST_CASE("LockFreeRingBuffer Correctness", "[LockFreeRingBuffer]") {
    LockFreeRingBuffer<float> buffer(512);

    REQUIRE(buffer.Capacity() == 512);
    REQUIRE(buffer.Empty());
    REQUIRE(!buffer.Full());
    REQUIRE(buffer.Occupancy() == 0);
    REQUIRE(buffer.Size() == 0);

    std::vector<float> dataIn(256, 1.0f);
    REQUIRE(buffer.Push(dataIn.data(), 256) == 256);
    REQUIRE(buffer.Occupancy() == 256);
    REQUIRE(!buffer.Empty());

    std::vector<float> dataOut(256, 0.0f);
    REQUIRE(buffer.Pop(dataOut.data(), 256) == 256);
    REQUIRE(buffer.Occupancy() == 0);
    REQUIRE(buffer.Empty());

    // Verify data
    for (float v : dataOut) {
        REQUIRE(v == 1.0f);
    }
}

TEST_CASE("LockFreeRingBuffer Wrap Around", "[LockFreeRingBuffer]") {
    LockFreeRingBuffer<float> buffer(10);
    std::vector<float> dataIn = {1, 2, 3, 4, 5, 6, 7, 8};
    buffer.Push(dataIn.data(), 8);
    
    std::vector<float> dataOut(5);
    buffer.Pop(dataOut.data(), 5); // Pop 5, leaves 3
    REQUIRE(buffer.Occupancy() == 3);

    // Push 5 more, will wrap around
    std::vector<float> dataIn2 = {9, 10, 11, 12, 13};
    buffer.Push(dataIn2.data(), 5);
    REQUIRE(buffer.Occupancy() == 8);

    std::vector<float> dataOut2(8);
    buffer.Pop(dataOut2.data(), 8);
    
    std::vector<float> expected = {6, 7, 8, 9, 10, 11, 12, 13};
    for (size_t i = 0; i < 8; ++i) {
        REQUIRE(dataOut2[i] == expected[i]);
    }
}

TEST_CASE("LockFreeRingBuffer Threaded Stress", "[LockFreeRingBuffer]") {
    LockFreeRingBuffer<int> buffer(1024);
    std::atomic<bool> running{true};
    std::atomic<long long> totalConsumed{0};
    
    std::thread producer([&]() {
        std::vector<int> block(128, 42);
        for(int i = 0; i < 10000; ++i) { // 1.28M items
            while(buffer.Push(block.data(), 128) != 128) {
                std::this_thread::yield();
            }
        }
        running = false;
    });

    std::thread consumer([&]() {
        std::vector<int> block(128);
        while(running || !buffer.Empty()) {
            size_t popped = buffer.Pop(block.data(), 128);
            totalConsumed += popped;
        }
    });

    producer.join();
    consumer.join();

    REQUIRE(totalConsumed == 10000 * 128);
}

TEST_CASE("LockFreeRingBuffer Performance Benchmark", "[benchmark][LockFreeRingBuffer]") {
    LockFreeRingBuffer<float> buffer(4096);
    std::vector<float> block(256, 1.0f);
    std::vector<float> out(256, 0.0f);

    BENCHMARK("Push and Pop 256 frames") {
        buffer.Push(block.data(), 256);
        buffer.Pop(out.data(), 256);
        return out[0];
    };
}
