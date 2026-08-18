#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include "../src/audio/SampleRateConverter.h"
#include <vector>
#include <cmath>

using namespace VoiceClear::Audio;

TEST_CASE("SampleRateConverter Passthrough", "[SampleRateConverter]") {
    SampleRateConverter src(nullptr);
    REQUIRE(src.Initialize(48000, 48000));

    std::vector<float> inputData = {0.1f, 0.2f, 0.3f, 0.4f};
    AudioBuffer inBuffer;
    inBuffer.samples = inputData;
    inBuffer.frames = 4;
    inBuffer.sampleRate = 48000;

    std::vector<float> outData(8, 0.0f);
    AudioBuffer outBuffer;
    outBuffer.samples = outData;

    REQUIRE(src.Convert(inBuffer, outBuffer));
    REQUIRE(outBuffer.frames == 4);
    REQUIRE(outBuffer.sampleRate == 48000);
    REQUIRE(outBuffer.samples[0] == 0.1f);
    REQUIRE(outBuffer.samples[3] == 0.4f);
}

TEST_CASE("SampleRateConverter 44.1kHz to 48kHz Up-sampling", "[SampleRateConverter]") {
    SampleRateConverter src(nullptr);
    REQUIRE(src.Initialize(44100, 48000));

    // 44.1kHz 10ms block -> 441 frames
    // 48kHz 10ms block -> 480 frames
    std::vector<float> inputData(441, 0.5f);
    AudioBuffer inBuffer;
    inBuffer.samples = inputData;
    inBuffer.frames = 441;
    inBuffer.sampleRate = 44100;

    std::vector<float> outData(500, 0.0f);
    AudioBuffer outBuffer;
    outBuffer.samples = outData;

    REQUIRE(src.Convert(inBuffer, outBuffer));
    // Up-sampling 441 frames by 48000/44100 = 1.0884... 
    // Expecting 480 output frames
    REQUIRE(outBuffer.frames == 480);
    REQUIRE(outBuffer.sampleRate == 48000);
}

TEST_CASE("SampleRateConverter Sine Wave Preservation Benchmark", "[benchmark][SampleRateConverter]") {
    SampleRateConverter src(nullptr);
    src.Initialize(44100, 48000);

    std::vector<float> sineWave(441);
    for (int i = 0; i < 441; ++i) {
        sineWave[i] = std::sin(2.0f * 3.14159f * 1000.0f * (static_cast<float>(i) / 44100.0f));
    }
    
    AudioBuffer inBuffer;
    inBuffer.samples = sineWave;
    inBuffer.frames = 441;
    inBuffer.sampleRate = 44100;

    std::vector<float> outData(480);
    AudioBuffer outBuffer;
    outBuffer.samples = outData;

    BENCHMARK("Convert 44.1 to 48kHz (Cubic Hermite)") {
        src.Convert(inBuffer, outBuffer);
        return outBuffer.samples[0];
    };
}
