#pragma once
#include <string>
#include <memory>
#include <onnxruntime_cxx_api.h>
#include "../diagnostics/TelemetryManager.h"

namespace VoiceClear::AI {

    struct ModelManifest {
        std::string version;
        std::string model_name;
        int sample_rate;
        int frame_size;
        std::string checksum;
    };

    /**
     * @brief Discovers and manages ONNX models.
     * Parses models.json, validates topology, and handles fallback.
     */
    class ModelManager {
    public:
        explicit ModelManager(std::shared_ptr<Diagnostics::TelemetryManager> telemetry);
        ~ModelManager() = default;

        /**
         * @brief Scans models.json and populates the manifest catalog.
         */
        void DiscoverModels();

        /**
         * @brief Loads the selected model into an ONNX Runtime Session.
         * Automatically rolls back to the last known good model if validation fails.
         * @return Unique pointer to the constructed session, or nullptr on failure.
         */
        std::unique_ptr<Ort::Session> LoadModel(const std::string& modelName);

        // Provides access to the globally shared ONNX Environment
        Ort::Env& GetEnv() { return *m_env; }

    private:
        std::shared_ptr<Diagnostics::TelemetryManager> m_telemetry;
        std::unique_ptr<Ort::Env> m_env;
        std::unique_ptr<Ort::SessionOptions> m_sessionOptions;

        std::vector<ModelManifest> m_catalog;
        std::string m_lastKnownGoodModel;

        void ConfigureSessionOptions();
        bool ValidateModel(Ort::Session& session, const ModelManifest& manifest);
        ModelManifest* FindManifest(const std::string& modelName);
    };

}
