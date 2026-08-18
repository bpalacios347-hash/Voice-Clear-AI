#include "ServiceMain.h"
#include "../utils/Logger.h"
#include "../utils/CrashReporter.h"
#include "../utils/AutoUpdater.h"
#include "../core/config/ConfigManager.h"
#include <iostream>

namespace VoiceClear::Service {

    ServiceMain& ServiceMain::GetInstance() {
        static ServiceMain instance;
        return instance;
    }

    // =========================================================================
    // Entry Point
    // =========================================================================

    int ServiceMain::Run(int argc, char** argv) {
        // --- Initialize crash reporter before anything else ---
        // (catches crashes even during startup)
        Utils::CrashReporter::Initialize(nullptr, true);

        // --- Logger ---
        Utils::Logger::GetInstance()->Log(Core::LogLevel::Info,
            "Voice Clear AI Service v1.0.0 starting...");

        if (argc > 1) {
            std::string arg = argv[1];
            if (arg == "--install") {
                InstallService();
                return 0;
            } else if (arg == "--uninstall") {
                UninstallService();
                return 0;
            } else if (arg == "--console") {
                Utils::Logger::GetInstance()->Log(Core::LogLevel::Info,
                    "Running in console mode");
                StartInternal();
                std::cout << "Service started in console mode. Press Enter to exit..." << std::endl;
                std::cin.get();
                StopInternal();
                return 0;
            }
        }

        SERVICE_TABLE_ENTRYA dispatchTable[] = {
            { const_cast<char*>(m_serviceName.c_str()), (LPSERVICE_MAIN_FUNCTIONA)ServiceMainCallback },
            { nullptr, nullptr }
        };

        if (!StartServiceCtrlDispatcherA(dispatchTable)) {
            DWORD err = GetLastError();
            if (err == ERROR_FAILED_SERVICE_CONTROLLER_CONNECT) {
                // Running as standalone / background process (not SCM)
                Utils::Logger::GetInstance()->Log(Core::LogLevel::Info,
                    "Running as standalone background engine.");
                StartInternal();
                
                while (true) {
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }
                StopInternal();
                return 0;
            }

            Utils::Logger::GetInstance()->Log(Core::LogLevel::Error,
                "StartServiceCtrlDispatcher failed. Error=" + std::to_string(err));
            return 1;
        }

        return 0;
    }

    // =========================================================================
    // SCM Install / Uninstall
    // =========================================================================

    void ServiceMain::InstallService() {
        char path[MAX_PATH];
        if (!GetModuleFileNameA(nullptr, path, MAX_PATH)) return;

        SC_HANDLE hSCM = OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
        if (!hSCM) {
            std::cerr << "Must run as Administrator to install service.\n";
            return;
        }

        // Set description via ChangeServiceConfig2
        SC_HANDLE hService = CreateServiceA(
            hSCM, m_serviceName.c_str(), "Voice Clear AI Core Engine",
            SERVICE_ALL_ACCESS,
            SERVICE_WIN32_OWN_PROCESS,
            SERVICE_AUTO_START,
            SERVICE_ERROR_NORMAL,
            path, nullptr, nullptr, nullptr, nullptr, nullptr);

        if (hService) {
            // Set description
            SERVICE_DESCRIPTIONA desc{};
            desc.lpDescription = const_cast<char*>(
                "Provides real-time AI-based noise cancellation via DeepFilterNet ONNX. "
                "Requires MMCSS Pro Audio scheduling and WASAPI Exclusive mode.");
            ChangeServiceConfig2A(hService, SERVICE_CONFIG_DESCRIPTION, &desc);

            // Failure recovery: restart after 1st and 2nd crash, then wait 5 min
            SC_ACTION actions[3] = {
                {SC_ACTION_RESTART, 5000},   // restart after 5 s
                {SC_ACTION_RESTART, 10000},  // restart after 10 s
                {SC_ACTION_NONE,    0}        // give up
            };
            SERVICE_FAILURE_ACTIONSA failureActions{};
            failureActions.dwResetPeriod = 86400; // 24 h
            failureActions.lpCommand     = nullptr;
            failureActions.lpRebootMsg   = nullptr;
            failureActions.cActions      = 3;
            failureActions.lpsaActions   = actions;
            ChangeServiceConfig2A(hService, SERVICE_CONFIG_FAILURE_ACTIONS, &failureActions);

            std::cout << "Service installed successfully.\n";
            CloseServiceHandle(hService);
        } else {
            std::cerr << "Failed to install service. Error: " << GetLastError() << "\n";
        }
        CloseServiceHandle(hSCM);
    }

    void ServiceMain::UninstallService() {
        SC_HANDLE hSCM = OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
        if (!hSCM) return;

        SC_HANDLE hService = OpenServiceA(hSCM, m_serviceName.c_str(), SERVICE_STOP | DELETE);
        if (hService) {
            SERVICE_STATUS status{};
            ControlService(hService, SERVICE_CONTROL_STOP, &status);
            Sleep(2000); // Give it time to stop
            if (DeleteService(hService)) {
                std::cout << "Service uninstalled successfully.\n";
            } else {
                std::cerr << "Failed to delete service. Error: " << GetLastError() << "\n";
            }
            CloseServiceHandle(hService);
        }
        CloseServiceHandle(hSCM);
    }

    // =========================================================================
    // SCM Callbacks
    // =========================================================================

    void WINAPI ServiceMain::ServiceMainCallback(DWORD /*argc*/, LPTSTR* /*argv*/) {
        auto& self = GetInstance();

        self.m_statusHandle = RegisterServiceCtrlHandlerExA(
            self.m_serviceName.c_str(), ServiceCtrlHandler, nullptr);
        if (!self.m_statusHandle) return;

        self.m_status.dwServiceType      = SERVICE_WIN32_OWN_PROCESS;
        self.m_status.dwCurrentState     = SERVICE_START_PENDING;
        self.m_status.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
        self.m_status.dwWaitHint         = 10000; // 10 s startup budget
        SetServiceStatus(self.m_statusHandle, &self.m_status);

        self.m_hStopEvent = CreateEventA(nullptr, TRUE, FALSE, nullptr);
        self.StartInternal();

        self.m_status.dwCurrentState = SERVICE_RUNNING;
        self.m_status.dwWaitHint     = 0;
        SetServiceStatus(self.m_statusHandle, &self.m_status);

        // Block until SCM triggers SERVICE_CONTROL_STOP
        if (self.m_hStopEvent) {
            WaitForSingleObject(self.m_hStopEvent, INFINITE);
            CloseHandle(self.m_hStopEvent);
            self.m_hStopEvent = nullptr;
        }
    }

    DWORD WINAPI ServiceMain::ServiceCtrlHandler(
        DWORD control, DWORD /*eventType*/, LPVOID /*eventData*/, LPVOID /*context*/)
    {
        auto& self = GetInstance();

        switch (control) {
            case SERVICE_CONTROL_STOP:
            case SERVICE_CONTROL_SHUTDOWN:
                self.m_status.dwCurrentState = SERVICE_STOP_PENDING;
                self.m_status.dwWaitHint     = 5000;
                SetServiceStatus(self.m_statusHandle, &self.m_status);

                self.StopInternal();

                if (self.m_hStopEvent) {
                    SetEvent(self.m_hStopEvent);
                }

                self.m_status.dwCurrentState = SERVICE_STOPPED;
                self.m_status.dwWin32ExitCode = NO_ERROR;
                SetServiceStatus(self.m_statusHandle, &self.m_status);
                return NO_ERROR;

            case SERVICE_CONTROL_INTERROGATE:
                SetServiceStatus(self.m_statusHandle, &self.m_status);
                return NO_ERROR;

            default:
                break;
        }
        return NO_ERROR;
    }

    // =========================================================================
    // StartInternal / StopInternal
    // =========================================================================

    void ServiceMain::StartInternal() {
        Utils::Logger::GetInstance()->Log(Core::LogLevel::Info,
            "Voice Clear Windows Service — StartInternal()");

        // Set working directory to executable directory so relative paths (models/, config/) resolve correctly
        char exePath[MAX_PATH] = {};
        if (GetModuleFileNameA(nullptr, exePath, MAX_PATH) > 0) {
            std::filesystem::path exeDir = std::filesystem::path(exePath).parent_path();
            SetCurrentDirectoryA(exeDir.string().c_str());
            Utils::Logger::GetInstance()->Log(Core::LogLevel::Info,
                "Working directory set to: " + exeDir.string());
        }

        // Load runtime configuration
        Core::Config::ConfigManager cfg;
        const bool cfgLoaded = cfg.Load("C:\\ProgramData\\VoiceClear\\settings.json");
        if (!cfgLoaded) {
            // Try the install directory fallback
            char installDir[MAX_PATH] = {};
            HKEY hKey = nullptr;
            if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\VoiceClearAI",
                               0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS) {
                DWORD sz = MAX_PATH;
                RegQueryValueExA(hKey, "InstallPath", nullptr, nullptr,
                                 (LPBYTE)installDir, &sz);
                RegCloseKey(hKey);
            }
            const std::string cfgFallback = std::string(installDir) + "\\config\\settings.json";
            cfg.Load(cfgFallback);
        }

        // --- Telemetry ---
        m_telemetry = std::make_shared<Diagnostics::TelemetryManager>();

        // Re-init crash reporter with live telemetry for richer sidecar files
        const bool uploadEnabled = cfg.GetValue("service.crash_upload_enabled", true);
        Utils::CrashReporter::Initialize(m_telemetry, uploadEnabled);

        // --- Engine ---
        m_engine = std::make_shared<EngineBootstrap>();
        if (!m_engine->Initialize()) {
            Utils::Logger::GetInstance()->Log(Core::LogLevel::Error,
                "EngineBootstrap::Initialize() failed. Service will run in degraded mode.");
        }

        // --- IPC Server ---
        m_ipcServer = std::make_shared<Diagnostics::IpcServer>(m_telemetry, m_engine);

        // --- Watchdog ---
        m_watchdog = std::make_shared<Watchdog>(m_telemetry, m_engine);

        // Start subsystems in order
        m_ipcServer->Start();
        m_engine->Start();
        m_watchdog->Start();

        // --- Check for updates (async, non-blocking) ---
        const bool autoUpdate = cfg.GetValue("updates.auto_check", true);
        if (autoUpdate) {
            Utils::AutoUpdater::CheckForUpdates("1.0.0",
                [](bool available, const Utils::UpdateManifest& m) {
                    if (available) {
                        Utils::Logger::GetInstance()->Log(Core::LogLevel::Info,
                            "[AutoUpdater] New version available: " + m.version +
                            " — " + m.releaseNotes);
                    }
                });
        }

        Utils::Logger::GetInstance()->Log(Core::LogLevel::Info,
            "Voice Clear AI Service fully started.");
    }

    void ServiceMain::StopInternal() {
        Utils::Logger::GetInstance()->Log(Core::LogLevel::Info,
            "Voice Clear Windows Service — StopInternal()");

        if (m_watchdog)  m_watchdog->Stop();
        if (m_ipcServer) m_ipcServer->Stop();
        if (m_engine)    m_engine->Stop();

        Utils::Logger::GetInstance()->Log(Core::LogLevel::Info,
            "Voice Clear AI Service stopped cleanly.");
    }

} // namespace VoiceClear::Service
