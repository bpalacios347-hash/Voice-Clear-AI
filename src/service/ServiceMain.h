#pragma once
#include <windows.h>
#include <string>
#include <memory>
#include "../EngineBootstrap.h"
#include "../diagnostics/IpcServer.h"
#include "Watchdog.h"

namespace VoiceClear::Service {

    /**
     * @brief Windows Service Control Manager (SCM) hook and Lifecycle owner.
     */
    class ServiceMain {
    public:
        static ServiceMain& GetInstance();

        // Entry point for the executable. Handles CLI flags and SCM dispatch.
        int Run(int argc, char** argv);

    private:
        ServiceMain() = default;
        ~ServiceMain() = default;

        ServiceMain(const ServiceMain&) = delete;
        ServiceMain& operator=(const ServiceMain&) = delete;

        // CLI Installers
        void InstallService();
        void UninstallService();

        // SCM Callbacks
        static void WINAPI ServiceMainCallback(DWORD argc, LPTSTR* argv);
        static DWORD WINAPI ServiceCtrlHandler(DWORD control, DWORD eventType, LPVOID eventData, LPVOID context);

        void StartInternal();
        void StopInternal();

        std::string m_serviceName{"VoiceClearService"};
        SERVICE_STATUS m_status{};
        SERVICE_STATUS_HANDLE m_statusHandle{};
        HANDLE m_hStopEvent{nullptr};

        // Core Systems
        std::shared_ptr<Diagnostics::TelemetryManager> m_telemetry;
        std::shared_ptr<EngineBootstrap> m_engine;
        std::shared_ptr<Diagnostics::IpcServer> m_ipcServer;
        std::shared_ptr<Watchdog> m_watchdog;
    };

}
