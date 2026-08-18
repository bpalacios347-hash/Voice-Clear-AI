#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <cmath>
#include <fstream>
#include <windows.h>
#include <wincrypt.h>
#pragma comment(lib, "advapi32.lib")

#include <nlohmann/json.hpp>
#include "../src/ai/DeepFilterNetProcessor.h"
#include "../src/ai/ModelManager.h"
#include "../src/utils/Logger.h"

using namespace VoiceClear;

std::string GetFileSHA256(const std::string& filepath) {
    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
    std::string sha256 = "";
    
    if (CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
        if (CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash)) {
            FILE* f = nullptr;
            fopen_s(&f, filepath.c_str(), "rb");
            if (f) {
                const int bufSize = 32768;
                std::vector<uint8_t> buffer(bufSize);
                size_t bytesRead;
                while ((bytesRead = fread(buffer.data(), 1, bufSize, f)) > 0) {
                    CryptHashData(hHash, buffer.data(), static_cast<DWORD>(bytesRead), 0);
                }
                fclose(f);
                
                DWORD hashLen = 0;
                DWORD cbHashLen = sizeof(DWORD);
                if (CryptGetHashParam(hHash, HP_HASHSIZE, (BYTE*)&hashLen, &cbHashLen, 0)) {
                    std::vector<BYTE> hash(hashLen);
                    if (CryptGetHashParam(hHash, HP_HASHVAL, hash.data(), &hashLen, 0)) {
                        char hex[3];
                        for (DWORD i = 0; i < hashLen; i++) {
                            snprintf(hex, sizeof(hex), "%02x", hash[i]);
                            sha256 += hex;
                        }
                    }
                }
            }
            CryptDestroyHash(hHash);
        }
        CryptReleaseContext(hProv, 0);
    }
    return sha256;
}

void GenerateNoisyWav(const std::string& filename, int sampleRate, int durationSeconds) {
    std::vector<float> samples(sampleRate * durationSeconds);
    std::default_random_engine generator;
    std::normal_distribution<float> distribution(0.0f, 0.1f);

    for (size_t i = 0; i < samples.size(); ++i) {
        float time = static_cast<float>(i) / sampleRate;
        float signal = 0.5f * std::sin(2.0f * 3.1415926535f * 440.0f * time) + 
                       0.2f * std::sin(2.0f * 3.1415926535f * 880.0f * time);
        float noise = distribution(generator);
        samples[i] = signal + noise;
    }

    drwav_data_format format;
    format.container = drwav_container_riff;
    format.format = DR_WAVE_FORMAT_IEEE_FLOAT;
    format.channels = 1;
    format.sampleRate = sampleRate;
    format.bitsPerSample = 32;

    drwav wav;
    if (drwav_init_file_write(&wav, filename.c_str(), &format, nullptr)) {
        drwav_write_pcm_frames(&wav, samples.size(), samples.data());
        drwav_uninit(&wav);
        std::cout << "Generated " << filename << std::endl;
    }
}

float CalculateEnergy(const std::vector<float>& samples) {
    float sum = 0.0f;
    for (float s : samples) {
        sum += s * s;
    }
    return sum;
}

int main(int argc, char** argv) {
    std::string inputFilename = "noisy_speech_48k.wav";
    std::string outputFilename = "enhanced_speech_48k.wav";
    std::string benchmarkFilename = "benchmark_results.json";

    // Parse arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--input" && i + 1 < argc) {
            inputFilename = argv[++i];
        } else if (arg == "--output" && i + 1 < argc) {
            outputFilename = argv[++i];
        } else if (arg == "--benchmark" && i + 1 < argc) {
            benchmarkFilename = argv[++i];
        }
    }

    // Generate test noisy file if needed
    GenerateNoisyWav(inputFilename, 48000, 5);

    // Read WAV
    unsigned int channels = 0, sampleRate = 0;
    drwav_uint64 totalPCMFrameCount = 0;
    float* pSampleData = drwav_open_file_and_read_pcm_frames_f32(inputFilename.c_str(), &channels, &sampleRate, &totalPCMFrameCount, nullptr);
    if (pSampleData == nullptr) {
        std::cerr << "Failed to read " << inputFilename << std::endl;
        return 1;
    }

    std::cout << "Read WAV: " << totalPCMFrameCount << " frames, " << channels << " ch, " << sampleRate << " Hz" << std::endl;

    std::vector<float> inputSamples(pSampleData, pSampleData + totalPCMFrameCount);
    drwav_free(pSampleData, nullptr);

    // Initialize AI
    auto modelManager = std::make_shared<AI::ModelManager>(nullptr);
    auto session = modelManager->LoadModel("deepfilternet3.onnx");
    if (!session) {
        std::cerr << "Failed to load deepfilternet3.onnx!" << std::endl;
        return 1;
    }



    std::string modelPath = "models/deepfilternet3.onnx";
    std::string modelSha256 = GetFileSHA256(modelPath);

    auto processor = std::make_unique<AI::DeepFilterNetProcessor>(nullptr);
    if (!processor->Initialize(std::move(session))) {
        std::cerr << "Processor failed to initialize model!" << std::endl;
        return 1;
    }

    std::vector<float> outputSamples;
    outputSamples.reserve(inputSamples.size());

    const size_t frameSize = 480; // 10ms at 48kHz
    size_t numFrames = inputSamples.size() / frameSize;

    std::cout << "Processing " << numFrames << " frames..." << std::endl;

    double totalMs = 0;
    double maxMs = 0;
    double minMs = 999999.0;
    std::vector<double> times;
    times.reserve(numFrames);

    float inputEnergy = CalculateEnergy(inputSamples);

    for (size_t i = 0; i < numFrames; ++i) {
        std::span<float> frameSpan(inputSamples.data() + i * frameSize, frameSize);
        VoiceClear::Audio::AudioBuffer frame;
        frame.samples = frameSpan;
        
        auto start = std::chrono::high_resolution_clock::now();
        processor->Process(frame);
        auto end = std::chrono::high_resolution_clock::now();

        double ms = std::chrono::duration<double, std::milli>(end - start).count();
        totalMs += ms;
        maxMs = std::max(maxMs, ms);
        minMs = std::min(minMs, ms);
        times.push_back(ms);

        outputSamples.insert(outputSamples.end(), frame.samples.begin(), frame.samples.end());
    }

    // Also test pure noise suppression across profiles
    std::vector<float> pureNoise(48000 * 2);
    std::default_random_engine gen;
    std::normal_distribution<float> dist(0.0f, 0.05f); // typical background noise
    for (auto& s : pureNoise) s = dist(gen);
    float noiseInEnergy = CalculateEnergy(pureNoise);

    std::cout << "\n=== PURE NOISE TESTS ===" << std::endl;
    for (auto& profile : {AI::NoiseProfile::Balanced(), AI::NoiseProfile::Strong()}) {
        processor->ResetState();
        processor->SetProfile(profile);

        std::vector<float> noiseTest = pureNoise;
        std::vector<float> noiseOut;
        noiseOut.reserve(noiseTest.size());
        for (size_t i = 0; i < noiseTest.size() / frameSize; ++i) {
            std::span<float> fSpan(noiseTest.data() + i * frameSize, frameSize);
            VoiceClear::Audio::AudioBuffer fBuf;
            fBuf.samples = fSpan;
            fBuf.frames = static_cast<uint32_t>(frameSize);
            fBuf.channels = 1;
            fBuf.sampleRate = 48000;
            processor->Process(fBuf);
            noiseOut.insert(noiseOut.end(), fBuf.samples.begin(), fBuf.samples.end());
        }
        float noiseOutEnergy = CalculateEnergy(noiseOut);
        float attDb = 10.0f * std::log10(noiseInEnergy / std::max(1e-9f, noiseOutEnergy));
        std::cout << "Profile [" << profile.name << "] Pure Noise In: " << noiseInEnergy 
                  << ", Out: " << noiseOutEnergy << " -> Attenuation: " << attDb << " dB" << std::endl;
    }


    // Write output WAV
    drwav_data_format format;
    format.container = drwav_container_riff;
    format.format = DR_WAVE_FORMAT_IEEE_FLOAT;
    format.channels = 1;
    format.sampleRate = 48000;
    format.bitsPerSample = 32;

    drwav wav;
    if (drwav_init_file_write(&wav, outputFilename.c_str(), &format, nullptr)) {
        drwav_write_pcm_frames(&wav, outputSamples.size(), outputSamples.data());
        drwav_uninit(&wav);
        std::cout << "Wrote " << outputFilename << std::endl;
    }

    std::sort(times.begin(), times.end());
    double p50 = times[static_cast<size_t>(times.size() * 0.5)];
    double p95 = times[static_cast<size_t>(times.size() * 0.95)];
    double p99 = times[static_cast<size_t>(times.size() * 0.99)];
    double avgMs = totalMs / numFrames;

    float outputEnergy = CalculateEnergy(outputSamples);
    float snr = 10.0f * std::log10(inputEnergy / (outputEnergy + 1e-9f));

    std::cout << "--- Metrics ---" << std::endl;
    std::cout << "Input Energy: " << inputEnergy << ", Output Energy: " << outputEnergy << std::endl;
    std::cout << "Energy Ratio (Input/Output): " << snr << " dB" << std::endl;
    if (std::abs(inputEnergy - outputEnergy) < 1.0f) {
        std::cerr << "WARNING: Input and Output energy are suspiciously similar. Is the model bypassing?" << std::endl;
    } else {
        std::cout << "SUCCESS: Output audio successfully modified by the AI model." << std::endl;
    }

    std::string onnxVersion = OrtGetApiBase()->GetVersionString();

    nlohmann::json benchmarkJson = {
        {"metrics", {
            {"average_ms", avgMs},
            {"min_ms", minMs},
            {"max_ms", maxMs},
            {"p50_ms", p50},
            {"p95_ms", p95},
            {"p99_ms", p99}
        }},
        {"processing_info", {
            {"total_blocks", numFrames},
            {"total_duration_ms", totalMs},
            {"block_size_samples", frameSize}
        }},
        {"environment", {
            {"onnxruntime_version", onnxVersion},
            {"model_sha256", modelSha256}
        }},
        {"audio_verification", {
            {"input_energy", inputEnergy},
            {"output_energy", outputEnergy},
            {"reduction_db", snr}
        }}
    };

    std::ofstream o(benchmarkFilename);
    o << std::setw(4) << benchmarkJson << std::endl;
    std::cout << "Saved benchmark to " << benchmarkFilename << std::endl;

    return 0;
}
