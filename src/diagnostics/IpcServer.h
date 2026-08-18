#pragma once
/**
 * @file IpcServer.h
 * @brief Named-pipe IPC server running inside VoiceClearService.exe.
 *
 * Responsibilities:
 *  - Accept one UI client at a time via the Named Pipe defined in IpcProtocol.h
 *  - Authenticate via binary Handshake (version negotiation)
 *  - Dispatch binary commands: SetProfile, Enable/DisableProcessing, ReloadModel, GetStatus
 *  - Broadcast binary TelemetryPayload at 20 Hz on a separate broadcaster thread
 *  - Send CommandResponse after every command execution
 */

#include <windows.h>
#include <string>
#include <thread>
#include <atomic>
#include <vector>
#include <chrono>
#include "../diagnostics/TelemetryManager.h"
#include "../EngineBootstrap.h"
#include "../core/ipc/IpcProtocol.h"

namespace VoiceClear::Diagnostics {

    /**
     * @brief Binary Named-Pipe IPC Server.
     *
     * All command handling is done in the listener thread.
     * Telemetry broadcast runs on a dedicated broadcaster thread to avoid
     * blocking the command receive loop.
     */
    class IpcServer {
    public:
        IpcServer(std::shared_ptr<Diagnostics::TelemetryManager> telemetry,
                  std::shared_ptr<EngineBootstrap> engine);
        ~IpcServer();

        bool Start();
        void Stop();

    private:
        std::shared_ptr<Diagnostics::TelemetryManager> m_telemetry;
        std::shared_ptr<EngineBootstrap>                m_engine;

        std::atomic<bool>  m_running{false};
        std::atomic<bool>  m_clientAuthenticated{false};
        std::thread        m_listenerThread;
        std::thread        m_broadcasterThread;

        // Current pipe handle.  Written by listener; read by broadcaster.
        // Protected by the fact that only one client is allowed at a time.
        std::atomic<HANDLE> m_hPipe{INVALID_HANDLE_VALUE};
        std::mutex          m_pipeWriteMutex;

        // Uptime tracking
        std::chrono::steady_clock::time_point m_startTime;

        // ----------------------------------------------------------------
        // Threads
        // ----------------------------------------------------------------
        void ListenerLoop();
        void BroadcasterLoop();

        // ----------------------------------------------------------------
        // Handshake
        // ----------------------------------------------------------------
        bool AuthenticateClient(HANDLE hPipe);

        // ----------------------------------------------------------------
        // Command Handlers — each returns true if the pipe should remain open
        // ----------------------------------------------------------------
        bool HandleSetProfile(HANDLE hPipe, uint32_t payloadSize);
        bool HandleEnableProcessing(HANDLE hPipe, bool enable);
        bool HandleReloadModel(HANDLE hPipe, uint32_t payloadSize);
        bool HandleGetStatus(HANDLE hPipe);
        bool HandleGetDeviceList(HANDLE hPipe);
        bool HandleSetDevices(HANDLE hPipe, uint32_t payloadSize);
        bool HandleSetMonitor(HANDLE hPipe, uint32_t payloadSize);
        bool HandleUnknown(HANDLE hPipe, IPC::MessageType type, uint32_t payloadSize);

        // ----------------------------------------------------------------
        // Low-level I/O helpers
        // ----------------------------------------------------------------
        bool WriteResponse(HANDLE hPipe,
                           const IPC::CommandResponsePayload& resp) noexcept;
        bool WriteTelemetry(HANDLE hPipe) noexcept;
        IPC::TelemetryPayload BuildTelemetry() noexcept;
    };

} // namespace VoiceClear::Diagnostics
