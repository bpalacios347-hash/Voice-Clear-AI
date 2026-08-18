#include <catch2/catch_test_macros.hpp>
#include "../src/audio/FormatConverter.h"
#include <vector>
#include <cmath>

using namespace VoiceClear::Audio;

TEST_CASE("FormatConverter Int16 Mono to Canonical", "[FormatConverter]") {
    FormatConverter converter(nullptr);
    REQUIRE(converter.Initialize(AudioSampleFormat::Int16, 1));

    std::vector<int16_t> input = {0, 16384, 32767, -32768};
    std::vector<float> outData(4, 0.0f);
    AudioBuffer outBuffer;
    outBuffer.samples = outData;

    REQUIRE(converter.Convert(reinterpret_cast<const uint8_t*>(input.data()), 4, outBuffer));
    REQUIRE(outBuffer.frames == 4);
    REQUIRE(outBuffer.channels == 1);
    
    REQUIRE(std::abs(outBuffer.samples[0] - 0.0f) < 0.001f);
    REQUIRE(std::abs(outBuffer.samples[1] - 0.5f) < 0.001f);
    REQUIRE(std::abs(outBuffer.samples[2] - 1.0f) < 0.001f);
    REQUIRE(std::abs(outBuffer.samples[3] - (-1.0f)) < 0.001f);
}

TEST_CASE("FormatConverter Float32 Stereo to Mono Canonical", "[FormatConverter]") {
    FormatConverter converter(nullptr);
    REQUIRE(converter.Initialize(AudioSampleFormat::Float32, 2));

    // L, R, L, R
    std::vector<float> input = {1.0f, -1.0f, 0.5f, 0.5f};
    std::vector<float> outData(2, 0.0f);
    AudioBuffer outBuffer;
    outBuffer.samples = outData;

    REQUIRE(converter.Convert(reinterpret_cast<const uint8_t*>(input.data()), 2, outBuffer));
    REQUIRE(outBuffer.frames == 2);
    REQUIRE(outBuffer.channels == 1);
    
    // (1.0 + -1.0) / 2 = 0.0
    REQUIRE(std::abs(outBuffer.samples[0] - 0.0f) < 0.0001f);
    // (0.5 + 0.5) / 2 = 0.5
    REQUIRE(std::abs(outBuffer.samples[1] - 0.5f) < 0.0001f);
}
