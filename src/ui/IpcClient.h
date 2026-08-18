#pragma once
/**
 * @file IpcClient.h
 * @brief Named-pipe IPC client running inside VoiceClear.exe (Qt UI process).
 *
 * Runs on a dedicated QThread.  The main Qt thread communicates with it via
 * Qt signals and slots (auto-queued across thread boundaries).
 *
 * Outbound commands (slots):
 *   sendSetProfile(), sendEnableProcessing(), sendReloadModel(), sendGetStatus()
 *
 * Inbound events (signals):
 *   serviceConnected(), serviceDisconnected(),
 *   telemetryReceived(), commandResponseReceived(), statusReceived()
 */

#include <QThread>
#include <QString>
#include <windows.h>
#include <atomic>
#include <mutex>
#include <vector>
#include "../core/ipc/IpcProtocol.h"

namespace VoiceClear::UI {

    class IpcClient : public QThread {
        Q_OBJECT

    public:
        explicit IpcClient(QObject* parent = nullptr);
        ~IpcClient() override;

        void stop();

    // ----------------------------------------------------------------
    // Signals — emitted from the IPC thread, received by the UI thread
    // ----------------------------------------------------------------
    signals:
        void serviceConnected();
        void serviceDisconnected();

        /// Emitted at ~20 Hz when the service streams telemetry.
        void telemetryReceived(
            float    cpu,
            float    mem,
            float    peakMs,
            float    p99Ms,
            uint32_t xruns,
            float    uptimeSec,
            uint8_t  driverMode,
            uint8_t  aiEnabled,
            QString  activeProfile,
            QString  activeModel,
            QString  currentInputName,
            QString  currentOutputName,
            float    inputRmsLevel,
            float    outputRmsLevel,
            uint8_t  monitorEnabled);

        /// Emitted after each command the client sent receives a response.
        void commandResponseReceived(
            uint8_t  originalCommand,
            uint8_t  status,
            QString  message);

        /// Emitted in response to a GetStatus command.
        void statusReceived(IPC::StatusResponsePayload status);

        /// Emitted in response to a GetDeviceList command.
        void deviceListReceived(
            QStringList inputNames,
            QStringList inputIds,
            QStringList outputNames,
            QStringList outputIds,
            QString     selectedInputId,
            QString     selectedOutputId);

    // ----------------------------------------------------------------
    // Slots — called from the UI thread, safe to call any time
    // ----------------------------------------------------------------
    public slots:
        void sendSetProfile(const QString& profileId,
                            const QString& modelId = QString());
        void sendEnableProcessing(bool enabled);
        void sendReloadModel(const QString& modelName);
        void sendGetStatus();
        void sendGetDeviceList();
        void sendSetDevices(const QString& inputId, const QString& outputId);
        void sendSetMonitor(bool enabled);

    protected:
        void run() override;

    private:
        std::atomic<bool>     m_running{true};
        std::atomic<uint32_t> m_negotiatedVersion{2};
        HANDLE                m_hPipe{INVALID_HANDLE_VALUE};
        mutable std::mutex    m_writeMutex; ///< Serialise outbound writes from UI thread

        // ----------------------------------------------------------------
        // Internal helpers
        // ----------------------------------------------------------------
        bool connectAndHandshake();
        void readLoop();

        /// Thread-safe binary write helper.
        bool writeRaw(const void* data, DWORD size) noexcept;
        /// Sends a header-only message (zero payload).
        bool sendCommand(IPC::MessageType type) noexcept;
    };

} // namespace VoiceClear::UI
