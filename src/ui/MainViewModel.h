#pragma once
/**
 * @file MainViewModel.h
 * @brief Qt ViewModel connecting the IPC layer to the QML interface.
 *
 * Exposes all engine state as Q_PROPERTYs for binding in main.qml and
 * provides Q_INVOKABLEs for QML to send commands back to the service.
 */

#include <QObject>
#include <QString>
#include <cstdint>
#include "IpcClient.h"
#include "SystemTrayManager.h"

#include <memory>
#include <QTimer>
#include "../EngineBootstrap.h"
#include "../diagnostics/TelemetryManager.h"

namespace VoiceClear::UI {

    class MainViewModel : public QObject {
        Q_OBJECT

        // ---- Connection state ----
        Q_PROPERTY(bool    serviceConnected    READ isServiceConnected    NOTIFY serviceConnectedChanged)

        // ---- Microphone toggle ----
        Q_PROPERTY(bool    isMicrophoneEnabled READ isMicrophoneEnabled   WRITE setMicrophoneEnabled
                                                                           NOTIFY microphoneEnabledChanged)

        // ---- Device / model info ----
        Q_PROPERTY(QString     currentMicrophone   READ currentMicrophone     NOTIFY currentMicrophoneChanged)
        Q_PROPERTY(QString     currentOutput       READ currentOutput         NOTIFY currentOutputChanged)
        Q_PROPERTY(QStringList inputDevices        READ inputDevices          NOTIFY devicesChanged)
        Q_PROPERTY(QStringList outputDevices       READ outputDevices         NOTIFY devicesChanged)
        Q_PROPERTY(int         selectedInputIndex  READ selectedInputIndex    NOTIFY devicesChanged)
        Q_PROPERTY(int         selectedOutputIndex READ selectedOutputIndex   NOTIFY devicesChanged)
        Q_PROPERTY(QString     currentModel        READ currentModel          NOTIFY currentModelChanged)
        Q_PROPERTY(QString     activeProfile       READ activeProfile         NOTIFY activeProfileChanged)

        // ---- Live Audio Levels & Monitoring ----
        Q_PROPERTY(qreal   inputLevel          READ inputLevel            NOTIFY inputLevelChanged)
        Q_PROPERTY(qreal   outputLevel         READ outputLevel           NOTIFY outputLevelChanged)
        Q_PROPERTY(bool    isMonitorEnabled    READ isMonitorEnabled      WRITE setMonitorEnabled NOTIFY monitorEnabledChanged)

        // ---- Driver mode ----
        Q_PROPERTY(int     driverMode          READ driverMode            NOTIFY telemetryUpdated)

        // ---- Diagnostics (telemetry) ----
        Q_PROPERTY(QString cpuUsage    READ cpuUsage    NOTIFY telemetryUpdated)
        Q_PROPERTY(QString memoryUsage READ memoryUsage NOTIFY telemetryUpdated)
        Q_PROPERTY(QString p99Latency  READ p99Latency  NOTIFY telemetryUpdated)
        Q_PROPERTY(QString xrunCount   READ xrunCount   NOTIFY telemetryUpdated)

    public:
        explicit MainViewModel(QObject* parent = nullptr);
        ~MainViewModel() override;

        // ---- Property accessors ----
        bool        isServiceConnected()   const;
        bool        isMicrophoneEnabled()  const;
        void        setMicrophoneEnabled(bool enabled);

        qreal       inputLevel()           const { return m_inputLevel; }
        qreal       outputLevel()          const { return m_outputLevel; }
        bool        isMonitorEnabled()     const { return m_monitorEnabled; }
        void        setMonitorEnabled(bool enabled);

        QString     currentMicrophone()    const;
        QString     currentOutput()        const;
        QStringList inputDevices()         const;
        QStringList outputDevices()        const;
        int         selectedInputIndex()   const;
        int         selectedOutputIndex()  const;

        QString     currentModel()         const;
        QString     activeProfile()        const;
        int         driverMode()           const;

        QString     cpuUsage()             const;
        QString     memoryUsage()          const;
        QString     p99Latency()           const;
        QString     xrunCount()            const;

        // ---- Commands (callable from QML) ----
        Q_INVOKABLE void toggleDiagnostics();
        Q_INVOKABLE void toggleMonitor();
        Q_INVOKABLE void setProfile(const QString& profileId);
        Q_INVOKABLE void setWindowVisible(bool visible);
        Q_INVOKABLE void reloadModel(const QString& modelName);
        Q_INVOKABLE void requestStatus();
        Q_INVOKABLE void refreshDevices();
        Q_INVOKABLE void selectInputDevice(int index);
        Q_INVOKABLE void selectOutputDevice(int index);

    signals:
        void serviceConnectedChanged();
        void microphoneEnabledChanged();
        void inputLevelChanged();
        void outputLevelChanged();
        void monitorEnabledChanged();
        void currentMicrophoneChanged();
        void currentOutputChanged();
        void devicesChanged();
        void currentModelChanged();
        void activeProfileChanged(const QString& profile);
        void telemetryUpdated();
        void showDiagnosticsPanel();
        void commandFeedback(const QString& message, bool success);

    private slots:
        void onServiceConnected();
        void onServiceDisconnected();
        void onTelemetryReceived(float cpu, float mem, float peakMs, float p99Ms,
                                  uint32_t xruns, float uptime,
                                  uint8_t driverMode, uint8_t aiEnabled,
                                  const QString& profile, const QString& model,
                                  const QString& currentInput, const QString& currentOutput,
                                  float inputRms, float outputRms, uint8_t monitorEnabled);
        void onCommandResponseReceived(uint8_t originalCommand,
                                        uint8_t status,
                                        const QString& message);
        void onDeviceListReceived(const QStringList& inNames, const QStringList& inIds,
                                  const QStringList& outNames, const QStringList& outIds,
                                  const QString& selectedInputId, const QString& selectedOutputId);

    private:
        IpcClient*         m_ipcClient{nullptr};
        SystemTrayManager* m_trayManager{nullptr};

        std::shared_ptr<Diagnostics::TelemetryManager> m_inProcessTelemetry;
        std::shared_ptr<EngineBootstrap>               m_inProcessEngine;
        QTimer*                                        m_pollTimer{nullptr};

        bool        m_serviceConnected{true};
        bool        m_micEnabled{true};
        bool        m_monitorEnabled{false};
        float       m_inputLevel{0.0f};
        float       m_outputLevel{0.0f};
        int         m_driverMode{0};
        QString     m_currentMicrophone{"Default Microphone"};
        QString     m_currentOutput{"CABLE Input (VB-Audio Virtual Cable)"};
        QStringList m_inputNames;
        QStringList m_inputIds;
        QStringList m_outputNames;
        QStringList m_outputIds;
        int         m_selectedInputIdx{0};
        int         m_selectedOutputIdx{0};
        QString     m_currentModel{"DeepFilterNet 3"};
        QString     m_activeProfile{"Balanced"};

        float    m_cpu{0.0f};
        float    m_mem{0.0f};
        float    m_p99{0.0f};
        uint32_t m_xruns{0};
    };

} // namespace VoiceClear::UI
