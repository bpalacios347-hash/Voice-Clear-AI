#include "ModelManager.h"
#include "../utils/Logger.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <nlohmann/json.hpp>
#include <windows.h>
#include <bcrypt.h>

#pragma comment(lib, "bcrypt.lib")

namespace fs = std::filesystem;

// ---------------------------------------------------------------------------
// SHA-256 file hash helper using Windows BCrypt API (no external dependency)
// ---------------------------------------------------------------------------
static std::string ComputeSha256File(const std::string& path) {
    // 1. Open file
    std::ifstream file(path, std::ios::binary);
    if (!file) return {};

    // 2. Open BCrypt algorithm provider
    BCRYPT_ALG_HANDLE hAlg = nullptr;
    if (!BCRYPT_SUCCESS(BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA256_ALGORITHM, nullptr, 0)))
        return {};

    // 3. Create hash object
    DWORD hashObjLen = 0, hashLen = 0, cbResult = 0;
    BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH, reinterpret_cast<PUCHAR>(&hashObjLen), sizeof(hashObjLen), &cbResult, 0);
    BCryptGetProperty(hAlg, BCRYPT_HASH_LENGTH,   reinterpret_cast<PUCHAR>(&hashLen),    sizeof(hashLen),    &cbResult, 0);

    std::vector<uint8_t> hashObj(hashObjLen);
    BCRYPT_HASH_HANDLE hHash = nullptr;
    BCryptCreateHash(hAlg, &hHash, hashObj.data(), hashObjLen, nullptr, 0, 0);

    // 4. Hash file in 64 KiB chunks
    std::vector<char> buf(65536);
    while (file.read(buf.data(), static_cast<std::streamsize>(buf.size())) || file.gcount() > 0) {
        BCryptHashData(hHash, reinterpret_cast<PUCHAR>(buf.data()),
                       static_cast<ULONG>(file.gcount()), 0);
    }

    // 5. Finalise
    std::vector<uint8_t> digest(hashLen);
    BCryptFinishHash(hHash, digest.data(), hashLen, 0);
    BCryptDestroyHash(hHash);
    BCryptCloseAlgorithmProvider(hAlg, 0);

    // 6. Convert to lowercase hex string
    std::ostringstream oss;
    for (uint8_t b : digest) oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b);
    return oss.str();
}

namespace VoiceClear::AI {

    ModelManager::ModelManager(std::shared_ptr<Diagnostics::TelemetryManager> telemetry)
        : m_telemetry(std::move(telemetry)) 
    {
        // Initialize global ONNX environment (Warning severity to prevent log spam)
        m_env = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "VoiceClearAI");
        ConfigureSessionOptions();
        DiscoverModels();
    }

    void ModelManager::ConfigureSessionOptions() {
        m_sessionOptions = std::make_unique<Ort::SessionOptions>();

        // Enable all graph optimizations
        m_sessionOptions->SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

        // Restrict to single-threaded sequential execution to guarantee deterministic latency 
        // avoiding thread-pool contention with the real-time audio thread
        m_sessionOptions->SetExecutionMode(ExecutionMode::ORT_SEQUENTIAL);
        m_sessionOptions->SetIntraOpNumThreads(1); 
        m_sessionOptions->SetInterOpNumThreads(1);
        
        // Memory pattern optimization
        m_sessionOptions->EnableMemPattern();
        m_sessionOptions->EnableCpuMemArena();

        // NOTE: AVX2 and oneDNN are automatically leveraged by the CPU Execution Provider if supported by the CPU
        // and compiled into the loaded ONNX Runtime library via vcpkg.
    }

    static fs::path GetModelsDirectory() {
        if (fs::exists("models/models.json")) {
            return fs::path("models");
        }
        char exePath[MAX_PATH] = {};
        if (GetModuleFileNameA(nullptr, exePath, MAX_PATH) > 0) {
            fs::path exeDir = fs::path(exePath).parent_path();
            if (fs::exists(exeDir / "models" / "models.json")) {
                return exeDir / "models";
            }
        }
        return fs::path("models");
    }

    void ModelManager::DiscoverModels() {
        m_catalog.clear();
        fs::path modelsDir = GetModelsDirectory();
        if (!fs::exists(modelsDir)) return;

        fs::path catalogPath = modelsDir / "models.json";
        if (!fs::exists(catalogPath)) return;

        try {
            std::ifstream file(catalogPath);
            nlohmann::json j;
            file >> j;

            for (const auto& item : j) {
                ModelManifest manifest;
                manifest.version = item.value("version", "");
                manifest.model_name = item.value("model_name", "");
                manifest.sample_rate = item.value("sample_rate", 48000);
                manifest.frame_size = item.value("frame_size", 480);
                manifest.checksum = item.value("checksum", "");
                m_catalog.push_back(manifest);
            }
        } catch (const std::exception& e) {
            Utils::Logger::GetInstance()->Log(Core::LogLevel::Error, "Failed to parse models.json: " + std::string(e.what()));
        }
    }

    ModelManifest* ModelManager::FindManifest(const std::string& modelName) {
        for (auto& manifest : m_catalog) {
            if (manifest.model_name == modelName) return &manifest;
        }
        return nullptr;
    }

    bool ModelManager::ValidateModel(Ort::Session& session, const ModelManifest& manifest) {
        // 1. Structural topology check
        if (session.GetInputCount() == 0 || session.GetOutputCount() == 0) {
            Utils::Logger::GetInstance()->Log(Core::LogLevel::Error,
                "[ModelManager] Model has zero inputs or outputs.");
            return false;
        }

        // 2. SHA-256 integrity verification
        if (!manifest.checksum.empty()) {
            fs::path modelPath = GetModelsDirectory() / manifest.model_name;
            std::string actualHash = ComputeSha256File(modelPath.string());
            if (actualHash.empty()) {
                Utils::Logger::GetInstance()->Log(Core::LogLevel::Error,
                    "[ModelManager] Failed to compute SHA-256 for: " + modelPath.string());
                return false;
            }
            // Compare case-insensitively
            std::string expected = manifest.checksum;
            std::transform(expected.begin(),   expected.end(),   expected.begin(),   ::tolower);
            std::transform(actualHash.begin(), actualHash.end(), actualHash.begin(), ::tolower);
            if (actualHash != expected) {
                Utils::Logger::GetInstance()->Log(Core::LogLevel::Error,
                    "[ModelManager] SHA-256 mismatch for " + manifest.model_name +
                    ". Expected: " + expected + " Got: " + actualHash);
                return false;
            }
            Utils::Logger::GetInstance()->Log(Core::LogLevel::Info,
                "[ModelManager] SHA-256 verified OK for: " + manifest.model_name);
        } else {
            Utils::Logger::GetInstance()->Log(Core::LogLevel::Warn,
                "[ModelManager] No checksum in manifest for " +
                manifest.model_name + " — skipping integrity check.");
        }

        return true;
    }

    std::unique_ptr<Ort::Session> ModelManager::LoadModel(const std::string& modelName) {
        auto manifest = FindManifest(modelName);
        if (!manifest) {
            std::cerr << "[ModelManager] Model not in models.json catalog: " << modelName << std::endl;
            Utils::Logger::GetInstance()->Log(Core::LogLevel::Error, "Model not in models.json catalog: " + modelName);
            return nullptr;
        }

        fs::path modelPath = GetModelsDirectory() / modelName;
        
        if (!fs::exists(modelPath)) {
            std::cerr << "[ModelManager] Model file not found: " << modelPath.string() << std::endl;
            Utils::Logger::GetInstance()->Log(Core::LogLevel::Error, "Model file not found: " + modelPath.string());
            return nullptr;
        }

        try {
            std::wstring wModelPath = modelPath.wstring();
            auto session = std::make_unique<Ort::Session>(*m_env, wModelPath.c_str(), *m_sessionOptions);
            
            if (!ValidateModel(*session, *manifest)) {
                std::cerr << "[ModelManager] ValidateModel failed for " << modelName << std::endl;
                throw std::runtime_error("Model topology validation failed.");
            }

            m_lastKnownGoodModel = modelName;
            std::cout << "[ModelManager] Successfully loaded model: " << modelName << std::endl;
            Utils::Logger::GetInstance()->Log(Core::LogLevel::Info, "Successfully loaded model: " + modelName);
            
            return session;
        } catch (const Ort::Exception& e) {
            std::cerr << "[ModelManager] Ort::Exception: " << e.what() << std::endl;
            Utils::Logger::GetInstance()->Log(Core::LogLevel::Error, "Failed to load model " + modelName + " : " + e.what());
            return nullptr;
        } catch (const std::exception& e) {
            std::cerr << "[ModelManager] std::exception: " << e.what() << std::endl;
            Utils::Logger::GetInstance()->Log(Core::LogLevel::Error, "Validation failed for " + modelName + " : " + e.what());
            return nullptr;
        }
    }

}
