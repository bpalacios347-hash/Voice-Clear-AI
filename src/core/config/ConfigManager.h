#pragma once
#include <string>
#include <nlohmann/json.hpp>

namespace VoiceClear::Core::Config {

    class ConfigManager {
    public:
        ConfigManager() = default;
        ~ConfigManager() = default;

        /**
         * @brief Load JSON configuration from the given file path.
         * @param filePath Path to the json file.
         * @return true if successfully loaded and parsed.
         */
        bool Load(const std::string& filePath);

        /**
         * @brief Get a value from the loaded configuration.
         */
        template <typename T>
        T GetValue(const std::string& key, const T& defaultValue) const {
            if (m_config.contains(key)) {
                return m_config[key].get<T>();
            }
            return defaultValue;
        }

    private:
        nlohmann::json m_config;
    };

}
