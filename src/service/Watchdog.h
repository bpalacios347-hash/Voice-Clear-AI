#pragma once
#include <memory>
#include <thread>
#include <atomic>
#include "../EngineBootstrap.h"
#include "../diagnostics/TelemetryManager.h"

namespace VoiceClear::Service {

    /**
     * @brief Supervises the entire Audio Engine. 
     * Handles graceful restarts internally without taking down the Windows Service.
     */
    class Watchdog {
    public:
        Watchdog(std::shared_ptr<Diagnostics::TelemetryManager> telemetry,
                 std::shared_ptr<EngineBootstrap> engine);
        ~Watchdog();

        bool Start();
        void Stop();

    private:
        std::shared_ptr<Diagnostics::TelemetryManager> m_telemetry;
        std::shared_ptr<EngineBootstrap> m_engine;

        std::atomic<bool> m_running{false};
        std::thread m_monitorThread;

        void MonitorLoop();
        void PerformGracefulRecovery();
    };

}
