/**
 * @file IpcClient.cpp
 * @brief Named-pipe IPC client implementation for the Qt UI process.
 *
 * Thread model:
 *   - QThread::run() owns the pipe and blocks in readLoop().
 *   - Outbound command slots are called from the Qt main thread and
 *     serialised via m_writeMutex.
 *   - All Qt signals are queued automatically across thread boundaries
 *     because IpcClient lives in the UI thread but executes in its own
 *     QThread context.
 */

#include "IpcClient.h"
#include <QDebug>
#include <cstring>
#include <tlhelp32.h>
#include <filesystem>

namespace VoiceClear::UI {

    using IPC::MessageHeader;
    using IPC::MessageType;
    using IPC::MakeHeader;

    // =========================================================================
    // Lifecycle
    // =========================================================================

    IpcClient::IpcClient(QObject* parent) : QThread(parent) {}

    IpcClient::~IpcClient() { stop(); }

    void IpcClient::stop() {
        m_running.store(false, std::memory_order_release);
        HANDLE h = m_hPipe;
        if (h != INVALID_HANDLE_VALUE) {
            // Closing the handle unblocks ReadFile in readLoop()
            CancelIoEx(h, nullptr);
            CloseHandle(h);
            m_hPipe = INVALID_HANDLE_VALUE;
        }
        wait();
    }

    // =========================================================================
    // QThread::run()
    // =========================================================================

    static void TryStartServiceIfNeeded() {
        bool serviceStarted = false;
        SC_HANDLE hSCM = OpenSCManagerA(nullptr, nullptr, SC_MANAGER_CONNECT);
        if (hSCM) {
            SC_HANDLE hSvc = OpenServiceA(hSCM, "VoiceClearService", SERVICE_START | SERVICE_QUERY_STATUS);
            if (hSvc) {
                SERVICE_STATUS_PROCESS ssp{};
                DWORD bytesNeeded = 0;
                if (QueryServiceStatusEx(hSvc, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp, sizeof(ssp), &bytesNeeded)) {
                    if (ssp.dwCurrentState == SERVICE_STOPPED) {
                        if (StartServiceA(hSvc, 0, nullptr)) {
                            serviceStarted = true;
                        }
                    } else if (ssp.dwCurrentState == SERVICE_RUNNING) {
                        serviceStarted = true;
                    }
                }
                CloseServiceHandle(hSvc);
            }
            CloseServiceHandle(hSCM);
        }

        if (!serviceStarted) {
            // Check if VoiceClearService process is already alive
            HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
            bool isProcessAlive = false;
            if (hSnap != INVALID_HANDLE_VALUE) {
                PROCESSENTRY32W pe{};
                pe.dwSize = sizeof(pe);
                if (Process32FirstW(hSnap, &pe)) {
                    do {
                        if (_wcsicmp(pe.szExeFile, L"VoiceClearService.exe") == 0) {
                            isProcessAlive = true;
                            break;
                        }
                    } while (Process32NextW(hSnap, &pe));
                }
                CloseHandle(hSnap);
            }

            if (!isProcessAlive) {
                char exePath[MAX_PATH] = {};
                if (GetModuleFileNameA(nullptr, exePath, MAX_PATH) > 0) {
                    std::filesystem::path dir = std::filesystem::path(exePath).parent_path();
                    std::filesystem::path svcExe = dir / "VoiceClearService.exe";
                    if (std::filesystem::exists(svcExe)) {
                        STARTUPINFOA si{};
                        PROCESS_INFORMATION pi{};
                        si.cb = sizeof(si);
                        std::string cmd = "\"" + svcExe.string() + "\"";
                        if (CreateProcessA(nullptr, cmd.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, dir.string().c_str(), &si, &pi)) {
                            if (pi.hProcess) CloseHandle(pi.hProcess);
                            if (pi.hThread) CloseHandle(pi.hThread);
                        }
                    }
                }
            }
        }
    }

    void IpcClient::run() {
        int retryCount = 0;
        while (m_running.load(std::memory_order_relaxed)) {
            if (connectAndHandshake()) {
                retryCount = 0;
                emit serviceConnected();
                readLoop();
                emit serviceDisconnected();
            } else {
                if (++retryCount % 2 == 1) {
                    TryStartServiceIfNeeded();
                }
            }
            // Reconnect with exponential backoff (cap at 2 s)
            static int delay = 250;
            QThread::msleep(delay);
            delay = std::min(delay * 2, 2000);
        }
    }

    // =========================================================================
    // connectAndHandshake
    // =========================================================================

    bool IpcClient::connectAndHandshake() {
        // Wait for the pipe to become available (non-blocking fast path first)
        HANDLE h = CreateFileA(
            IPC::PIPE_NAME,
            GENERIC_READ | GENERIC_WRITE,
            0, nullptr, OPEN_EXISTING, 0, nullptr);

        if (h == INVALID_HANDLE_VALUE) {
            DWORD err = GetLastError();
            if (err == ERROR_PIPE_BUSY) {
                // Server is busy with another client; wait up to 500 ms
                WaitNamedPipeA(IPC::PIPE_NAME, 500);
            }
            return false;
        }

        // Switch to byte-read mode (matches server)
        DWORD mode = PIPE_READMODE_BYTE;
        SetNamedPipeHandleState(h, &mode, nullptr, nullptr);

        m_hPipe = h;

        // Try negotiating latest version first, then backwards compatible versions
        uint32_t versionsToTry[] = { IPC::PROTOCOL_VERSION, 3, 2 };
        bool authenticated = false;

        for (uint32_t ver : versionsToTry) {
            IPC::HandshakeRequest req{};
            req.protocolVersion = ver;
            req.clientType      = IPC::ClientType::GUI;

            MessageHeader outHdr = MakeHeader(MessageType::Handshake, sizeof(req));
            outHdr.protocolVersion = ver;

            DWORD bw = 0;
            if (!WriteFile(h, &outHdr, sizeof(outHdr), &bw, nullptr) ||
                !WriteFile(h, &req,    sizeof(req),    &bw, nullptr)) {
                break;
            }

            MessageHeader inHdr{};
            DWORD br = 0;
            if (!ReadFile(h, &inHdr, sizeof(inHdr), &br, nullptr) ||
                br != sizeof(inHdr) ||
                inHdr.type != MessageType::Handshake) {
                break;
            }

            IPC::HandshakeResponse res{};
            if (ReadFile(h, &res, sizeof(res), &br, nullptr) && res.accepted) {
                authenticated = true;
                m_negotiatedVersion = ver;
                break;
            }
        }

        if (!authenticated) {
            CloseHandle(h);
            m_hPipe = INVALID_HANDLE_VALUE;
            return false;
        }

        return true;
    }

    // =========================================================================
    // readLoop
    // =========================================================================

    void IpcClient::readLoop() {
        while (m_running.load(std::memory_order_relaxed)) {
            HANDLE h = m_hPipe;
            if (h == INVALID_HANDLE_VALUE) break;

            MessageHeader hdr{};
            DWORD br = 0;
            if (!ReadFile(h, &hdr, sizeof(hdr), &br, nullptr) ||
                br != sizeof(hdr)) break;

            if (hdr.protocolVersion < 2 || hdr.protocolVersion > IPC::PROTOCOL_VERSION) break;

            switch (hdr.type) {

                case MessageType::TelemetryBinary: {
                    if (hdr.payloadSize == sizeof(IPC::TelemetryPayload)) {
                        IPC::TelemetryPayload p{};
                        if (!ReadFile(h, &p, sizeof(p), &br, nullptr)) goto disconnect;
                        emit telemetryReceived(
                            p.cpuUsage, p.memoryUsageMb,
                            p.peakLatencyMs, p.p99LatencyMs,
                            p.xrunCount, p.uptimeSeconds,
                            p.driverMode, p.aiEnabled,
                            QString::fromLatin1(p.activeProfile, strnlen(p.activeProfile, sizeof(p.activeProfile))),
                            QString::fromLatin1(p.activeModel,   strnlen(p.activeModel,   sizeof(p.activeModel))),
                            QString::fromUtf8(p.currentInputName,  strnlen(p.currentInputName,  sizeof(p.currentInputName))),
                            QString::fromUtf8(p.currentOutputName, strnlen(p.currentOutputName, sizeof(p.currentOutputName))),
                            p.inputRmsLevel,
                            p.outputRmsLevel,
                            p.monitorEnabled);
                    } else {
                        #pragma pack(push, 1)
                        struct TelemetryPayloadV2 {
                            float       cpuUsage;
                            float       memoryUsageMb;
                            float       peakLatencyMs;
                            float       p99LatencyMs;
                            uint32_t    xrunCount;
                            float       uptimeSeconds;
                            uint8_t     driverMode;
                            uint8_t     aiEnabled;
                            char        activeProfile[32];
                            char        activeModel[128];
                        } p2{};
                        #pragma pack(pop)

                        if (hdr.payloadSize == sizeof(p2)) {
                            if (!ReadFile(h, &p2, sizeof(p2), &br, nullptr)) goto disconnect;
                            emit telemetryReceived(
                                p2.cpuUsage, p2.memoryUsageMb,
                                p2.peakLatencyMs, p2.p99LatencyMs,
                                p2.xrunCount, p2.uptimeSeconds,
                                p2.driverMode, p2.aiEnabled,
                                QString::fromLatin1(p2.activeProfile, strnlen(p2.activeProfile, sizeof(p2.activeProfile))),
                                QString::fromLatin1(p2.activeModel,   strnlen(p2.activeModel,   sizeof(p2.activeModel))),
                                QString(), QString(),
                                0.0f, 0.0f, 0);
                        } else if (hdr.payloadSize > 0) {
                            std::vector<char> discard(hdr.payloadSize);
                            ReadFile(h, discard.data(), hdr.payloadSize, &br, nullptr);
                        }
                    }
                    break;
                }

                case MessageType::DeviceListResponse: {
                    if (hdr.payloadSize != sizeof(IPC::DeviceListPayload)) break;
                    IPC::DeviceListPayload dl{};
                    if (!ReadFile(h, &dl, sizeof(dl), &br, nullptr)) goto disconnect;
                    QStringList inNames, inIds, outNames, outIds;
                    for (uint32_t i = 0; i < dl.deviceCount && i < 32; i++) {
                        const auto& dev = dl.devices[i];
                        QString name = QString::fromUtf8(dev.friendlyName, strnlen(dev.friendlyName, sizeof(dev.friendlyName)));
                        QString id   = QString::fromUtf8(dev.id, strnlen(dev.id, sizeof(dev.id)));
                        if (dev.isCapture) {
                            inNames.append(name);
                            inIds.append(id);
                        } else {
                            outNames.append(name);
                            outIds.append(id);
                        }
                    }
                    QString selIn = QString::fromUtf8(dl.selectedInputId, strnlen(dl.selectedInputId, sizeof(dl.selectedInputId)));
                    QString selOut = QString::fromUtf8(dl.selectedOutputId, strnlen(dl.selectedOutputId, sizeof(dl.selectedOutputId)));
                    emit deviceListReceived(inNames, inIds, outNames, outIds, selIn, selOut);
                    break;
                }

                case MessageType::CommandResponse: {
                    if (hdr.payloadSize != sizeof(IPC::CommandResponsePayload)) break;
                    IPC::CommandResponsePayload resp{};
                    if (!ReadFile(h, &resp, sizeof(resp), &br, nullptr)) goto disconnect;
                    emit commandResponseReceived(
                        static_cast<uint8_t>(resp.originalCommand),
                        static_cast<uint8_t>(resp.status),
                        QString::fromLatin1(resp.message, strnlen(resp.message, sizeof(resp.message))));
                    break;
                }

                case MessageType::StatusResponse: {
                    if (hdr.payloadSize != sizeof(IPC::StatusResponsePayload)) break;
                    IPC::StatusResponsePayload snap{};
                    if (!ReadFile(h, &snap, sizeof(snap), &br, nullptr)) goto disconnect;
                    emit statusReceived(snap);
                    break;
                }

                case MessageType::ShutdownUI: {
                    // Server is shutting down; exit cleanly
                    goto disconnect;
                }

                default: {
                    // Unknown / future message; drain payload to keep pipe in sync
                    if (hdr.payloadSize > 0 && hdr.payloadSize <= 65536) {
                        std::vector<char> discard(hdr.payloadSize);
                        ReadFile(h, discard.data(), hdr.payloadSize, &br, nullptr);
                    }
                    break;
                }
            }
            continue;

        disconnect:
            break;
        }

        HANDLE h = m_hPipe;
        if (h != INVALID_HANDLE_VALUE) {
            CloseHandle(h);
            m_hPipe = INVALID_HANDLE_VALUE;
        }
    }

    // =========================================================================
    // Outbound Command Slots
    // =========================================================================

    void IpcClient::sendSetProfile(const QString& profileId, const QString& modelId) {
        uint32_t ver = m_negotiatedVersion.load();
        IPC::SetProfilePayload p{};
        p.protocolVersion = ver;
        p.timestamp       = GetTickCount64();

        QByteArray pBytes = profileId.toLatin1();
        strncpy_s(p.profileId, pBytes.constData(), sizeof(p.profileId) - 1);

        if (!modelId.isEmpty()) {
            QByteArray mBytes = modelId.toLatin1();
            strncpy_s(p.modelId, mBytes.constData(), sizeof(p.modelId) - 1);
        }

        MessageHeader hdr = MakeHeader(MessageType::SetProfile, sizeof(p));
        hdr.protocolVersion = ver;
        std::lock_guard<std::mutex> lk(m_writeMutex);
        writeRaw(&hdr, sizeof(hdr));
        writeRaw(&p,   sizeof(p));
    }

    void IpcClient::sendEnableProcessing(bool enabled) {
        sendCommand(enabled ? MessageType::EnableProcessing : MessageType::DisableProcessing);
    }

    void IpcClient::sendReloadModel(const QString& modelName) {
        uint32_t ver = m_negotiatedVersion.load();
        IPC::ReloadModelPayload p{};
        QByteArray b = modelName.toLatin1();
        strncpy_s(p.modelName, b.constData(), sizeof(p.modelName) - 1);

        MessageHeader hdr = MakeHeader(MessageType::ReloadModel, sizeof(p));
        hdr.protocolVersion = ver;
        std::lock_guard<std::mutex> lk(m_writeMutex);
        writeRaw(&hdr, sizeof(hdr));
        writeRaw(&p,   sizeof(p));
    }

    void IpcClient::sendGetStatus() {
        sendCommand(MessageType::GetStatus);
    }

    void IpcClient::sendGetDeviceList() {
        sendCommand(MessageType::GetDeviceList);
    }

    void IpcClient::sendSetDevices(const QString& inputId, const QString& outputId) {
        uint32_t ver = m_negotiatedVersion.load();
        IPC::SetDevicesPayload p{};
        QByteArray inBytes = inputId.toUtf8();
        QByteArray outBytes = outputId.toUtf8();
        strncpy_s(p.inputId, inBytes.constData(), sizeof(p.inputId) - 1);
        strncpy_s(p.outputId, outBytes.constData(), sizeof(p.outputId) - 1);

        MessageHeader hdr = MakeHeader(MessageType::SetDevices, sizeof(p));
        hdr.protocolVersion = ver;
        std::lock_guard<std::mutex> lk(m_writeMutex);
        writeRaw(&hdr, sizeof(hdr));
        writeRaw(&p,   sizeof(p));
    }

    void IpcClient::sendSetMonitor(bool enabled) {
        uint32_t ver = m_negotiatedVersion.load();
        IPC::SetMonitorPayload p{};
        p.enabled = enabled ? 1 : 0;

        MessageHeader hdr = MakeHeader(MessageType::SetMonitor, sizeof(p));
        hdr.protocolVersion = ver;
        std::lock_guard<std::mutex> lk(m_writeMutex);
        writeRaw(&hdr, sizeof(hdr));
        writeRaw(&p,   sizeof(p));
    }

    // =========================================================================
    // Low-level helpers
    // =========================================================================

    bool IpcClient::writeRaw(const void* data, DWORD size) noexcept {
        HANDLE h = m_hPipe;
        if (h == INVALID_HANDLE_VALUE) return false;
        DWORD bytesWritten = 0;
        return WriteFile(h, data, size, &bytesWritten, nullptr) && (bytesWritten == size);
    }

    bool IpcClient::sendCommand(IPC::MessageType type) noexcept {
        MessageHeader hdr = MakeHeader(type, 0);
        hdr.protocolVersion = m_negotiatedVersion.load();
        std::lock_guard<std::mutex> lock(m_writeMutex);
        return writeRaw(&hdr, sizeof(hdr));
    }

} // namespace VoiceClear::UI
