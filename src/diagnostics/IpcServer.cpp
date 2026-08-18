/**
 * @file IpcServer.cpp
 * @brief Named-pipe IPC server implementation.
 *
 * Design notes:
 *  - One pipe instance, one UI client at a time (MAX_PIPE_INSTANCES = 1).
 *  - ListenerLoop creates the pipe, waits for a client, authenticates, then
 *    dispatches binary commands in a tight read loop.
 *  - BroadcasterLoop pushes TelemetryPayload at 20 Hz while a client is
 *    authenticated.  Uses non-blocking PeekNamedPipe so it never blocks the
 *    broadcaster on client reads.
 *  - Each command sends a CommandResponse so the UI always knows the outcome.
 *  - Security: DACL grants GENERIC_READ|WRITE to NT AUTHORITY\Authenticated Users only.
 */

#include "IpcServer.h"
#include "../utils/Logger.h"
#include <sddl.h>   // ConvertStringSecurityDescriptorToSecurityDescriptor
#include <psapi.h>  // GetProcessMemoryInfo / PROCESS_MEMORY_COUNTERS_EX
#include <cassert>

#pragma comment(lib, "psapi.lib")

namespace VoiceClear::Diagnostics {

    using IPC::MessageHeader;
    using IPC::MessageType;
    using IPC::MakeHeader;
    using IPC::MakeSuccess;
    using IPC::MakeFailure;
    using IPC::PIPE_NAME;
    using IPC::TELEMETRY_INTERVAL_MS;

    // =========================================================================
    // Lifecycle
    // =========================================================================

    IpcServer::IpcServer(std::shared_ptr<Diagnostics::TelemetryManager> telemetry,
                         std::shared_ptr<EngineBootstrap> engine)
        : m_telemetry(std::move(telemetry))
        , m_engine(std::move(engine))
        , m_startTime(std::chrono::steady_clock::now())
    {}

    IpcServer::~IpcServer() {
        Stop();
    }

    bool IpcServer::Start() {
        if (m_running.load(std::memory_order_relaxed)) return true;

        m_running.store(true, std::memory_order_release);
        m_startTime = std::chrono::steady_clock::now();
        m_listenerThread    = std::thread(&IpcServer::ListenerLoop,    this);
        m_broadcasterThread = std::thread(&IpcServer::BroadcasterLoop, this);

        Utils::Logger::GetInstance()->Log(Core::LogLevel::Info,
            "[IpcServer] Started on Named Pipe: " + std::string(PIPE_NAME));
        return true;
    }

    void IpcServer::Stop() {
        if (!m_running.exchange(false)) return;

        // Wake up the blocking ConnectNamedPipe call by connecting a dummy client.
        HANDLE hDummy = CreateFileA(PIPE_NAME, GENERIC_WRITE, 0, nullptr,
                                    OPEN_EXISTING, 0, nullptr);
        if (hDummy != INVALID_HANDLE_VALUE) CloseHandle(hDummy);

        if (m_listenerThread.joinable())    m_listenerThread.join();
        if (m_broadcasterThread.joinable()) m_broadcasterThread.join();

        HANDLE h = m_hPipe.exchange(INVALID_HANDLE_VALUE);
        if (h != INVALID_HANDLE_VALUE) {
            DisconnectNamedPipe(h);
            CloseHandle(h);
        }

        Utils::Logger::GetInstance()->Log(Core::LogLevel::Info, "[IpcServer] Stopped.");
    }

    // =========================================================================
    // ListenerLoop
    // =========================================================================

    void IpcServer::ListenerLoop() {
        // --- Build a DACL that allows Authenticated Users only ---
        PSECURITY_DESCRIPTOR pSD = nullptr;
        SECURITY_ATTRIBUTES sa{};
        sa.nLength = sizeof(sa);
        sa.bInheritHandle = FALSE;

        // SDDL: D:(A;;GRGW;;;WD)  — Allow Generic Read + Write to Everyone / All Users
        const BOOL ok = ConvertStringSecurityDescriptorToSecurityDescriptorA(
            "D:(A;;GRGW;;;WD)", SDDL_REVISION_1, &pSD, nullptr);
        sa.lpSecurityDescriptor = ok ? pSD : nullptr;

        while (m_running.load(std::memory_order_acquire)) {
            HANDLE hPipe = CreateNamedPipeA(
                PIPE_NAME,
                PIPE_ACCESS_DUPLEX,
                PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
                IPC::MAX_PIPE_INSTANCES,
                4096,
                4096,
                0,
                &sa
            );

            if (hPipe == INVALID_HANDLE_VALUE) {
                Utils::Logger::GetInstance()->Log(Core::LogLevel::Error,
                    "[IpcServer] CreateNamedPipe failed: " + std::to_string(GetLastError()));
                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue;
            }

            // Blocking wait for a client
            BOOL connected = ConnectNamedPipe(hPipe, nullptr)
                           || (GetLastError() == ERROR_PIPE_CONNECTED);

            if (!m_running.load(std::memory_order_relaxed)) {
                CloseHandle(hPipe);
                break;
            }

            if (!connected) {
                CloseHandle(hPipe);
                continue;
            }

            m_hPipe.store(hPipe, std::memory_order_release);

            if (!AuthenticateClient(hPipe)) {
                m_hPipe.store(INVALID_HANDLE_VALUE, std::memory_order_release);
                DisconnectNamedPipe(hPipe);
                CloseHandle(hPipe);
                continue;
            }

            m_clientAuthenticated.store(true, std::memory_order_release);
            Utils::Logger::GetInstance()->Log(Core::LogLevel::Info,
                "[IpcServer] Client connected and authenticated.");

            // ---- Command dispatch loop ----
            while (m_running.load(std::memory_order_relaxed)) {
                DWORD bytesAvail = 0;
                if (!PeekNamedPipe(hPipe, nullptr, 0, nullptr, &bytesAvail, nullptr)) break;

                if (bytesAvail < sizeof(MessageHeader)) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(5));
                    continue;
                }

                MessageHeader hdr;
                DWORD bytesRead = 0;
                if (!ReadFile(hPipe, &hdr, sizeof(hdr), &bytesRead, nullptr)
                    || bytesRead != sizeof(hdr)) break;

                if (hdr.protocolVersion < 2 || hdr.protocolVersion > IPC::PROTOCOL_VERSION) {
                    auto resp = MakeFailure(hdr.type, IPC::CommandStatus::VersionMismatch,
                                           "Protocol version mismatch");
                    WriteResponse(hPipe, resp);
                    break;
                }

                bool keepGoing = true;
                switch (hdr.type) {
                    case MessageType::SetProfile:
                        keepGoing = HandleSetProfile(hPipe, hdr.payloadSize);
                        break;
                    case MessageType::EnableProcessing:
                        keepGoing = HandleEnableProcessing(hPipe, true);
                        break;
                    case MessageType::DisableProcessing:
                        keepGoing = HandleEnableProcessing(hPipe, false);
                        break;
                    case MessageType::ReloadModel:
                        keepGoing = HandleReloadModel(hPipe, hdr.payloadSize);
                        break;
                    case MessageType::GetStatus:
                        keepGoing = HandleGetStatus(hPipe);
                        break;
                    case MessageType::GetDeviceList:
                        keepGoing = HandleGetDeviceList(hPipe);
                        break;
                    case MessageType::SetDevices:
                        keepGoing = HandleSetDevices(hPipe, hdr.payloadSize);
                        break;
                    case MessageType::SetMonitor:
                        keepGoing = HandleSetMonitor(hPipe, hdr.payloadSize);
                        break;
                    default:
                        keepGoing = HandleUnknown(hPipe, hdr.type, hdr.payloadSize);
                        break;
                }

                if (!keepGoing) break;
            }

            m_clientAuthenticated.store(false, std::memory_order_release);
            m_hPipe.store(INVALID_HANDLE_VALUE, std::memory_order_release);
            DisconnectNamedPipe(hPipe);
            CloseHandle(hPipe);
            Utils::Logger::GetInstance()->Log(Core::LogLevel::Info,
                "[IpcServer] Client disconnected.");
        }

        if (pSD) LocalFree(pSD);
    }

    // =========================================================================
    // BroadcasterLoop
    // =========================================================================

    void IpcServer::BroadcasterLoop() {
        while (m_running.load(std::memory_order_acquire)) {
            std::this_thread::sleep_for(
                std::chrono::milliseconds(TELEMETRY_INTERVAL_MS));

            if (!m_clientAuthenticated.load(std::memory_order_acquire)) continue;

            HANDLE h = m_hPipe.load(std::memory_order_acquire);
            if (h == INVALID_HANDLE_VALUE) continue;

            WriteTelemetry(h);
        }
    }

    // =========================================================================
    // Handshake
    // =========================================================================

    bool IpcServer::AuthenticateClient(HANDLE hPipe) {
        MessageHeader hdr;
        DWORD bytesRead = 0;

        if (!ReadFile(hPipe, &hdr, sizeof(hdr), &bytesRead, nullptr)
            || bytesRead != sizeof(hdr)) return false;

        if (hdr.type != MessageType::Handshake
            || hdr.payloadSize != sizeof(IPC::HandshakeRequest)) return false;

        IPC::HandshakeRequest req{};
        if (!ReadFile(hPipe, &req, sizeof(req), &bytesRead, nullptr)) return false;

        const bool accepted = (req.protocolVersion >= 2 && req.protocolVersion <= IPC::PROTOCOL_VERSION);

        IPC::HandshakeResponse res{};
        res.protocolVersion = req.protocolVersion;
        res.accepted        = accepted ? 1 : 0;

        MessageHeader outHdr = MakeHeader(MessageType::Handshake, sizeof(res));
        DWORD bw = 0;
        WriteFile(hPipe, &outHdr, sizeof(outHdr), &bw, nullptr);
        WriteFile(hPipe, &res,    sizeof(res),    &bw, nullptr);

        return accepted;
    }

    // =========================================================================
    // Command Handlers
    // =========================================================================

    bool IpcServer::HandleSetProfile(HANDLE hPipe, uint32_t payloadSize) {
        if (payloadSize != sizeof(IPC::SetProfilePayload)) {
            auto resp = MakeFailure(MessageType::SetProfile,
                                    IPC::CommandStatus::InvalidProfile,
                                    "Invalid payload size");
            return WriteResponse(hPipe, resp);
        }

        IPC::SetProfilePayload cmd{};
        DWORD br = 0;
        if (!ReadFile(hPipe, &cmd, sizeof(cmd), &br, nullptr)
            || br != sizeof(cmd)) return false;

        // Ensure null-termination even if sender misbehaves
        cmd.profileId[sizeof(cmd.profileId) - 1] = '\0';

        bool ok = false;
        if (m_engine) {
            m_engine->SetProfile(cmd.profileId);
            ok = true;
        }

        auto resp = ok
            ? MakeSuccess(MessageType::SetProfile, cmd.profileId)
            : MakeFailure(MessageType::SetProfile,
                          IPC::CommandStatus::InvalidProfile,
                          "Engine not available");
        return WriteResponse(hPipe, resp);
    }

    bool IpcServer::HandleEnableProcessing(HANDLE hPipe, bool enable) {
        if (m_engine) m_engine->SetAiEnabled(enable);

        auto resp = MakeSuccess(
            enable ? MessageType::EnableProcessing : MessageType::DisableProcessing,
            enable ? "AI Enabled" : "AI Disabled");
        return WriteResponse(hPipe, resp);
    }

    bool IpcServer::HandleReloadModel(HANDLE hPipe, uint32_t payloadSize) {
        if (payloadSize != sizeof(IPC::ReloadModelPayload)) {
            auto resp = MakeFailure(MessageType::ReloadModel,
                                    IPC::CommandStatus::ModelLoadError,
                                    "Invalid payload size");
            return WriteResponse(hPipe, resp);
        }

        IPC::ReloadModelPayload cmd{};
        DWORD br = 0;
        if (!ReadFile(hPipe, &cmd, sizeof(cmd), &br, nullptr)
            || br != sizeof(cmd)) return false;

        cmd.modelName[sizeof(cmd.modelName) - 1] = '\0';

        bool ok = false;
        if (m_engine) {
            ok = m_engine->ReloadModel(cmd.modelName);
        }

        auto resp = ok
            ? MakeSuccess(MessageType::ReloadModel, cmd.modelName)
            : MakeFailure(MessageType::ReloadModel,
                          IPC::CommandStatus::ModelLoadError,
                          "Model reload failed");
        return WriteResponse(hPipe, resp);
    }

    bool IpcServer::HandleGetStatus(HANDLE hPipe) {
        IPC::StatusResponsePayload status{};
        status.telemetry  = BuildTelemetry();
        status.serviceState = 1; // Running

        MessageHeader outHdr = MakeHeader(MessageType::StatusResponse, sizeof(status));
        DWORD bw = 0;
        WriteFile(hPipe, &outHdr,   sizeof(outHdr),   &bw, nullptr);
        WriteFile(hPipe, &status,   sizeof(status),   &bw, nullptr);
        return true;
    }

    bool IpcServer::HandleGetDeviceList(HANDLE hPipe) {
        IPC::DeviceListPayload payload{};
        if (m_engine) {
            payload = m_engine->GetDeviceListPayload();
        }

        MessageHeader outHdr = MakeHeader(MessageType::DeviceListResponse, sizeof(payload));
        DWORD bw = 0;
        WriteFile(hPipe, &outHdr,   sizeof(outHdr),   &bw, nullptr);
        WriteFile(hPipe, &payload,  sizeof(payload),  &bw, nullptr);
        return true;
    }

    bool IpcServer::HandleSetDevices(HANDLE hPipe, uint32_t payloadSize) {
        if (payloadSize != sizeof(IPC::SetDevicesPayload)) {
            return HandleUnknown(hPipe, IPC::MessageType::SetDevices, payloadSize);
        }

        IPC::SetDevicesPayload req{};
        DWORD br = 0;
        if (!ReadFile(hPipe, &req, sizeof(req), &br, nullptr)) return false;

        bool ok = false;
        if (m_engine) {
            ok = m_engine->SetAudioDevices(req.inputId, req.outputId);
        }

        auto resp = ok
            ? MakeSuccess(IPC::MessageType::SetDevices, "Audio devices updated successfully")
            : MakeFailure(IPC::MessageType::SetDevices, IPC::CommandStatus::InvalidDevice, "Failed to update audio devices");
        return WriteResponse(hPipe, resp);
    }

    bool IpcServer::HandleSetMonitor(HANDLE hPipe, uint32_t payloadSize) {
        if (payloadSize != sizeof(IPC::SetMonitorPayload)) {
            auto resp = MakeFailure(MessageType::SetMonitor, IPC::CommandStatus::Failed, "Payload size mismatch");
            return WriteResponse(hPipe, resp);
        }
        IPC::SetMonitorPayload p{};
        DWORD bytesRead = 0;
        if (!ReadFile(hPipe, &p, sizeof(p), &bytesRead, nullptr)) return false;

        if (m_engine) {
            m_engine->SetMonitorEnabled(p.enabled != 0);
        }
        auto resp = MakeSuccess(MessageType::SetMonitor, "Monitor status updated");
        return WriteResponse(hPipe, resp);
    }

    bool IpcServer::HandleUnknown(HANDLE hPipe, IPC::MessageType type, uint32_t payloadSize) {
        // Drain the payload so the pipe stays synchronised
        if (payloadSize > 0 && payloadSize <= 65536) {
            std::vector<char> discard(payloadSize);
            DWORD br = 0;
            ReadFile(hPipe, discard.data(), payloadSize, &br, nullptr);
        }
        auto resp = MakeFailure(type, IPC::CommandStatus::UnknownCommand, "Unrecognised command");
        return WriteResponse(hPipe, resp);
    }

    // =========================================================================
    // I/O Helpers
    // =========================================================================

    bool IpcServer::WriteResponse(HANDLE hPipe,
                                  const IPC::CommandResponsePayload& resp) noexcept {
        std::lock_guard<std::mutex> lock(m_pipeWriteMutex);
        MessageHeader hdr = MakeHeader(MessageType::CommandResponse, sizeof(resp));
        DWORD bw = 0;
        if (!WriteFile(hPipe, &hdr,  sizeof(hdr),  &bw, nullptr)) return false;
        if (!WriteFile(hPipe, &resp, sizeof(resp), &bw, nullptr)) return false;
        return true;
    }

    bool IpcServer::WriteTelemetry(HANDLE hPipe) noexcept {
        auto payload = BuildTelemetry();
        MessageHeader hdr = MakeHeader(MessageType::TelemetryBinary, sizeof(payload));
        DWORD bw = 0;
        std::lock_guard<std::mutex> lock(m_pipeWriteMutex);
        if (!WriteFile(hPipe, &hdr,     sizeof(hdr),     &bw, nullptr)) return false;
        if (!WriteFile(hPipe, &payload, sizeof(payload), &bw, nullptr)) return false;
        return true;
    }

    IPC::TelemetryPayload IpcServer::BuildTelemetry() noexcept {
        IPC::TelemetryPayload payload{};

        if (m_telemetry) {
            auto snap = m_telemetry->GetSnapshot();
            payload.cpuUsage       = snap.currentCpuUsage;
            payload.peakLatencyMs  = snap.peakLatencyMs;
            payload.p99LatencyMs   = snap.currentLatencyMs;
            payload.xrunCount      = snap.xrunCount;
            payload.inputRmsLevel  = snap.inputRmsLevel;
            payload.outputRmsLevel = snap.outputRmsLevel;

            // Approximate working-set size via Win32 PSAPI
            PROCESS_MEMORY_COUNTERS pmc{};
            pmc.cb = sizeof(pmc);
            if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
                payload.memoryUsageMb = static_cast<float>(pmc.WorkingSetSize) / (1024.0f * 1024.0f);
            }
        }

        // Uptime
        auto now = std::chrono::steady_clock::now();
        payload.uptimeSeconds = std::chrono::duration<float>(now - m_startTime).count();

        if (m_engine) {
            payload.driverMode     = m_engine->GetDriverMode();
            payload.aiEnabled      = m_engine->IsAiEnabled() ? 1 : 0;
            payload.monitorEnabled = m_engine->IsMonitorEnabled() ? 1 : 0;

            // Active profile / model names
            const char* prof = m_engine->GetActiveProfileName();
            const char* mdl  = m_engine->GetActiveModelName();
            if (prof) strncpy_s(payload.activeProfile, prof, sizeof(payload.activeProfile) - 1);
            if (mdl)  strncpy_s(payload.activeModel,   mdl,  sizeof(payload.activeModel) - 1);

            // Active device names
            const char* inName = m_engine->GetCurrentInputDeviceName();
            const char* outName = m_engine->GetCurrentOutputDeviceName();
            if (inName)  strncpy_s(payload.currentInputName,  inName,  sizeof(payload.currentInputName) - 1);
            if (outName) strncpy_s(payload.currentOutputName, outName, sizeof(payload.currentOutputName) - 1);
        }

        return payload;
    }

} // namespace VoiceClear::Diagnostics
