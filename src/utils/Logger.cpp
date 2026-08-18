#include "Logger.h"
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <windows.h>
#include <filesystem>
#include <iostream>

namespace VoiceClear::Utils {

    Logger::Logger() {
        try {
            char appData[MAX_PATH];
            GetEnvironmentVariableA("ProgramData", appData, MAX_PATH);
            std::string logDir = std::string(appData) + "\\VoiceClear\\Logs";
            std::filesystem::create_directories(logDir);

            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            std::shared_ptr<spdlog::sinks::rotating_file_sink_mt> rotating_sink = nullptr;

            try {
                // 5 MB limit, 3 rotated files maximum
                rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                    logDir + "\\VoiceClear.log", 1024 * 1024 * 5, 3);
            } catch (...) {
                // If file is locked by another process, continue with console sink only
            }

            std::shared_ptr<spdlog::logger> logger;
            if (rotating_sink) {
                logger = std::make_shared<spdlog::logger>("VoiceClear", spdlog::sinks_init_list{console_sink, rotating_sink});
            } else {
                logger = std::make_shared<spdlog::logger>("VoiceClear", console_sink);
            }
            
            spdlog::set_default_logger(logger);
            
#ifdef NDEBUG
            spdlog::set_level(spdlog::level::info);
#else
            spdlog::set_level(spdlog::level::debug);
#endif
            spdlog::flush_on(spdlog::level::warn);
        } catch (...) {
            // Fallback safe: never allow Logger construction to throw
        }
    }

    void Logger::Log(Core::LogLevel level, const std::string& message, const std::source_location& location) {
        try {
            std::string loc_str = std::format("[{}:{}] ", location.file_name(), location.line());
            switch (level) {
                case Core::LogLevel::Trace: spdlog::trace("{}{}", loc_str, message); break;
                case Core::LogLevel::Debug: spdlog::debug("{}{}", loc_str, message); break;
                case Core::LogLevel::Info: spdlog::info("{}{}", loc_str, message); break;
                case Core::LogLevel::Warn: spdlog::warn("{}{}", loc_str, message); break;
                case Core::LogLevel::Error: spdlog::error("{}{}", loc_str, message); break;
                case Core::LogLevel::Critical: spdlog::critical("{}{}", loc_str, message); break;
            }
        } catch (...) {}
    }

    std::shared_ptr<Logger> Logger::GetInstance() {
        static std::shared_ptr<Logger> instance = std::make_shared<Logger>();
        return instance;
    }

}
