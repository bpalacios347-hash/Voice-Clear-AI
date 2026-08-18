#include "Watchdog.h"
#include "../utils/Logger.h"
#include <chrono>

namespace VoiceClear::Service {

    Watchdog::Watchdog(std::shared_ptr<Diagnostics::TelemetryManager> telemetry,
                       std::shared_ptr<EngineBootstrap> engine)
        : m_telemetry(std::move(telemetry)), m_engine(std::move(engine)) {}

    Watchdog::~Watchdog() {
        Stop();
    }

    bool Watchdog::Start() {
        if (m_running.load()) return true;

        m_running.store(true);
        m_monitorThread = std::thread(&Watchdog::MonitorLoop, this);
        Utils::Logger::GetInstance()->Log(Core::LogLevel::Info, "Watchdog started.");
        return true;
    }

    void Watchdog::Stop() {
        if (!m_running.load()) return;
        m_running.store(false);
        if (m_monitorThread.joinable()) {
            m_monitorThread.join();
        }
        Utils::Logger::GetInstance()->Log(Core::LogLevel::Info, "Watchdog stopped.");
    }

    void Watchdog::MonitorLoop() {
        // Basic health check loop
        while (m_running.load()) {
            std::this_thread::sleep_for(std::chrono::seconds(2));

            if (!m_engine) continue;

            // In production, we check explicit health flags from DriverBridge and AudioPipeline.
            // Recover only if massive unrecoverable cascade occurs (> 100,000 continuous XRUNs)
            if (m_telemetry && m_telemetry->GetSnapshot().xrunCount > 100000) {
                Utils::Logger::GetInstance()->Log(Core::LogLevel::Warn, "Watchdog: Critical XRUN cascade detected. Initiating graceful recovery.");
                PerformGracefulRecovery();
            }
        }
    }

    void Watchdog::PerformGracefulRecovery() {
        if (!m_engine) return;

        m_engine->Stop();
        
        // Clear telemetry anomaly that triggered the restart
        if (m_telemetry) {
            m_telemetry->Reset();
        }

        std::this_thread::sleep_for(std::chrono::seconds(1)); // Allow resources to free

        if (m_engine->Initialize()) {
            m_engine->Start();
            Utils::Logger::GetInstance()->Log(Core::LogLevel::Info, "Watchdog: Graceful recovery successful.");
        } else {
            Utils::Logger::GetInstance()->Log(Core::LogLevel::Error, "Watchdog: Graceful recovery failed. Service may require hard restart.");
        }
    }

}
