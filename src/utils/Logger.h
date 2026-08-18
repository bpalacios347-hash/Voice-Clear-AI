#pragma once
#include "core/interfaces/ILogger.h"
#include <memory>
#include <spdlog/spdlog.h>

namespace VoiceClear::Utils {

    class Logger : public Core::ILogger {
    public:
        Logger();
        ~Logger() override = default;

        void Log(Core::LogLevel level, 
                 const std::string& message, 
                 const std::source_location& location = std::source_location::current()) override;
                 
        static std::shared_ptr<Logger> GetInstance();
    };

}
