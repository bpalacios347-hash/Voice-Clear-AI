#pragma once

#include <string>
#include <format>
#include <source_location>

namespace VoiceClear::Core {

    enum class LogLevel {
        Trace,
        Debug,
        Info,
        Warn,
        Error,
        Critical
    };

    /**
     * @brief Logger interface to wrap spdlog and prevent direct dependency.
     */
    class ILogger {
    public:
        virtual ~ILogger() = default;

        /**
         * @brief Log a message with a specific level.
         * @param level The log level severity.
         * @param message The message to log.
         * @param location The source code location.
         */
        virtual void Log(LogLevel level, 
                         const std::string& message, 
                         const std::source_location& location = std::source_location::current()) = 0;
    };

} // namespace VoiceClear::Core
