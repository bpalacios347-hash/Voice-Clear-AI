#include "ConfigManager.h"
#include <fstream>
#include <iostream>

namespace VoiceClear::Core::Config {

    bool ConfigManager::Load(const std::string& filePath) {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            return false;
        }

        try {
            file >> m_config;
            return true;
        } catch (const nlohmann::json::parse_error& e) {
            std::cerr << "JSON parse error in " << filePath << ": " << e.what() << std::endl;
            return false;
        }
    }

}
