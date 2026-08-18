#pragma once

#include <string>

namespace VoiceClear::Core {

    /**
     * @brief Base interface for all system modules (Audio Engine, IPC, Device Monitor).
     * Follows Clean Architecture to allow lifecycle management by the Windows Service.
     */
    class IModule {
    public:
        virtual ~IModule() = default;

        /**
         * @brief Initialize the module.
         * @return true if successful, false otherwise.
         */
        virtual bool Initialize() = 0;

        /**
         * @brief Start the module's execution (e.g., spawn threads).
         * @return true if successful, false otherwise.
         */
        virtual bool Start() = 0;

        /**
         * @brief Stop the module's execution cleanly.
         */
        virtual void Stop() = 0;

        /**
         * @brief Get the name of the module.
         * @return Module name as string.
         */
        virtual std::string GetName() const = 0;
    };

} // namespace VoiceClear::Core
