#include "MainViewModel.h"
#include <QApplication>
#include "../audio/DeviceManager.h"

namespace VoiceClear::UI {

    // =========================================================================
    // Construction
    // =========================================================================

    MainViewModel::MainViewModel(QObject* parent) : QObject(parent) {
        m_trayManager = new SystemTrayManager(this);

        // ---- Tray -> ViewModel ----
        connect(m_trayManager, &SystemTrayManager::profileSelected,
                this, &MainViewModel::setProfile);

        connect(m_trayManager, &SystemTrayManager::enableProcessing,
                this, &MainViewModel::setMicrophoneEnabled);

        connect(m_trayManager, &SystemTrayManager::toggleWindowRequested,
                this, &MainViewModel::toggleDiagnostics);

        connect(m_trayManager, &SystemTrayManager::quitRequested,
                qApp, &QApplication::quit);

        // Initial device list scan
        refreshDevices();

        // Start In-Process AI Audio Engine
        m_inProcessTelemetry = std::make_shared<Diagnostics::TelemetryManager>();
        m_inProcessEngine = std::make_shared<EngineBootstrap>();
        if (m_inProcessEngine->Initialize()) {
            m_inProcessEngine->Start();
            m_serviceConnected = true;
            m_trayManager->updateState(true, m_driverMode, m_micEnabled ? 1 : 0);
            emit serviceConnectedChanged();
        }

        // 30 FPS Telemetry & VU Meters Poller (Optimized)
        m_pollTimer = new QTimer(this);
        connect(m_pollTimer, &QTimer::timeout, this, [this]() {
            if (!m_inProcessTelemetry) return;
            auto snap = m_inProcessTelemetry->GetSnapshot();
            m_cpu = snap.currentCpuUsage;
            m_p99 = snap.currentLatencyMs;
            m_xruns = snap.xrunCount;
            m_driverMode = m_inProcessEngine ? m_inProcessEngine->GetDriverMode() : 0;

            float newInput = std::max(snap.inputRmsLevel, m_inputLevel * 0.75f);
            float newOutput = std::max(snap.outputRmsLevel, m_outputLevel * 0.75f);
            if (newInput < 0.005f) newInput = 0.0f;
            if (newOutput < 0.005f) newOutput = 0.0f;

            if (std::abs(newInput - m_inputLevel) > 0.002f) {
                m_inputLevel = newInput;
                emit inputLevelChanged();
            }
            if (std::abs(newOutput - m_outputLevel) > 0.002f) {
                m_outputLevel = newOutput;
                emit outputLevelChanged();
            }
            emit telemetryUpdated();
        });
        m_pollTimer->start(33); // 30 FPS active GUI rendering
    }

    void MainViewModel::setWindowVisible(bool visible) {
        if (!m_pollTimer) return;
        if (visible) {
            m_pollTimer->setInterval(33); // 30 FPS active GUI rendering
        } else {
            m_pollTimer->setInterval(500); // 2 Hz background tray polling (0.0% CPU)
        }
    }

    MainViewModel::~MainViewModel() {
        if (m_pollTimer) {
            m_pollTimer->stop();
        }
        if (m_inProcessEngine) {
            m_inProcessEngine->Stop();
        }
    }

    // =========================================================================
    // Property Accessors
    // =========================================================================

    bool        MainViewModel::isServiceConnected()   const { return m_serviceConnected; }
    bool        MainViewModel::isMicrophoneEnabled()  const { return m_micEnabled; }
    QString     MainViewModel::currentMicrophone()    const { return m_currentMicrophone; }
    QString     MainViewModel::currentOutput()        const { return m_currentOutput; }
    QStringList MainViewModel::inputDevices()         const { return m_inputNames; }
    QStringList MainViewModel::outputDevices()        const { return m_outputNames; }
    int         MainViewModel::selectedInputIndex()   const { return m_selectedInputIdx; }
    int         MainViewModel::selectedOutputIndex()  const { return m_selectedOutputIdx; }

    QString     MainViewModel::currentModel()         const { return m_currentModel; }
    QString     MainViewModel::activeProfile()        const { return m_activeProfile; }
    int         MainViewModel::driverMode()           const { return m_driverMode; }

    QString     MainViewModel::cpuUsage()             const { return QString::number(m_cpu,  'f', 1) + " %"; }
    QString     MainViewModel::memoryUsage()          const { return QString::number(m_mem,  'f', 1) + " MB"; }
    QString     MainViewModel::p99Latency()           const { return QString::number(m_p99,  'f', 2) + " ms"; }
    QString     MainViewModel::xrunCount()            const { return QString::number(m_xruns); }

    // =========================================================================
    // Setters
    // =========================================================================

    void MainViewModel::setMicrophoneEnabled(bool enabled) {
        if (m_micEnabled == enabled) return;
        m_micEnabled = enabled;
        emit microphoneEnabledChanged();

        if (m_inProcessEngine) {
            m_inProcessEngine->SetAiEnabled(enabled);
        }
        m_trayManager->updateState(m_serviceConnected, m_driverMode, m_micEnabled ? 1 : 0);
    }

    // =========================================================================
    // Q_INVOKABLEs (called from QML)
    // =========================================================================

    void MainViewModel::toggleDiagnostics() {
        emit showDiagnosticsPanel();
    }

    void MainViewModel::setProfile(const QString& profileId) {
        if (m_activeProfile != profileId) {
            m_activeProfile = profileId;
            emit activeProfileChanged(m_activeProfile);
        }
        if (m_inProcessEngine) {
            m_inProcessEngine->SetProfile(profileId.toUtf8().constData());
        }
    }

    void MainViewModel::reloadModel(const QString& modelName) {
        if (m_inProcessEngine) {
            m_inProcessEngine->ReloadModel(modelName.toUtf8().constData());
        }
        m_currentModel = modelName;
        emit currentModelChanged();
    }

    void MainViewModel::requestStatus() {
        // In-process status is always fresh
    }

    void MainViewModel::refreshDevices() {
        // Local WASAPI enumeration
        Audio::DeviceManager dm;
        dm.Initialize();
        auto inDevs = dm.EnumerateInputDevices();
        auto outDevs = dm.EnumerateOutputDevices();

        if (!inDevs.empty()) {
            m_inputNames.clear();
            m_inputIds.clear();
            int preferredPhysicalMicIdx = -1;
            for (const auto& d : inDevs) {
                if (d.state == Audio::DeviceState::Active) {
                    m_inputNames.append(QString::fromStdString(d.friendlyName));
                    m_inputIds.append(QString::fromStdString(d.id));

                    // Avoid virtual cable or loopback devices as default capture microphone
                    bool isVirtualOrLoopback = (d.friendlyName.find("CABLE") != std::string::npos ||
                                                d.friendlyName.find("Cable") != std::string::npos ||
                                                d.friendlyName.find("VB-Audio") != std::string::npos ||
                                                d.friendlyName.find("Stereo Mix") != std::string::npos ||
                                                d.friendlyName.find("Mezcla") != std::string::npos);
                    if (!isVirtualOrLoopback && preferredPhysicalMicIdx < 0) {
                        preferredPhysicalMicIdx = m_inputNames.size() - 1;
                    }
                }
            }
            if (m_selectedInputIdx >= m_inputNames.size() || m_selectedInputIdx < 0) {
                m_selectedInputIdx = (preferredPhysicalMicIdx >= 0) ? preferredPhysicalMicIdx : 0;
            } else {
                QString curName = m_inputNames.at(m_selectedInputIdx);
                if (curName.contains("CABLE", Qt::CaseInsensitive) || curName.contains("Stereo Mix", Qt::CaseInsensitive) || curName.contains("Mezcla", Qt::CaseInsensitive)) {
                    if (preferredPhysicalMicIdx >= 0) m_selectedInputIdx = preferredPhysicalMicIdx;
                }
            }
            if (!m_inputNames.isEmpty()) m_currentMicrophone = m_inputNames.at(m_selectedInputIdx);
        }

        if (!outDevs.empty()) {
            m_outputNames.clear();
            m_outputIds.clear();
            int cableIdx = -1;
            for (const auto& d : outDevs) {
                if (d.state == Audio::DeviceState::Active) {
                    m_outputNames.append(QString::fromStdString(d.friendlyName));
                    m_outputIds.append(QString::fromStdString(d.id));
                    if (d.friendlyName.find("CABLE") != std::string::npos || d.friendlyName.find("Cable") != std::string::npos) {
                        cableIdx = m_outputNames.size() - 1;
                    }
                }
            }
            if (cableIdx >= 0) {
                m_selectedOutputIdx = cableIdx;
            } else if (m_selectedOutputIdx >= m_outputNames.size()) {
                m_selectedOutputIdx = 0;
            }
            if (!m_outputNames.isEmpty()) m_currentOutput = m_outputNames.at(m_selectedOutputIdx);
        }

        emit currentMicrophoneChanged();
        emit currentOutputChanged();
        emit devicesChanged();
    }

    void MainViewModel::selectInputDevice(int index) {
        if (index < 0 || index >= m_inputIds.size()) return;
        m_selectedInputIdx = index;
        m_currentMicrophone = m_inputNames.at(index);
        emit currentMicrophoneChanged();
        emit devicesChanged();

        if (m_inProcessEngine) {
            std::string inId = m_inputIds.at(index).toStdString();
            std::string outId = (m_selectedOutputIdx >= 0 && m_selectedOutputIdx < m_outputIds.size())
                ? m_outputIds.at(m_selectedOutputIdx).toStdString() : "";
            m_inProcessEngine->SetAudioDevices(inId, outId);
        }
    }

    void MainViewModel::selectOutputDevice(int index) {
        if (index < 0 || index >= m_outputIds.size()) return;
        m_selectedOutputIdx = index;
        m_currentOutput = m_outputNames.at(index);
        emit currentOutputChanged();
        emit devicesChanged();

        if (m_inProcessEngine) {
            std::string inId = (m_selectedInputIdx >= 0 && m_selectedInputIdx < m_inputIds.size())
                ? m_inputIds.at(m_selectedInputIdx).toStdString() : "";
            std::string outId = m_outputIds.at(index).toStdString();
            m_inProcessEngine->SetAudioDevices(inId, outId);
        }
    }

    void MainViewModel::toggleMonitor() {
        setMonitorEnabled(!m_monitorEnabled);
    }

    void MainViewModel::setMonitorEnabled(bool enabled) {
        if (m_monitorEnabled != enabled) {
            m_monitorEnabled = enabled;
            emit monitorEnabledChanged();
            if (m_inProcessEngine) {
                m_inProcessEngine->SetMonitorEnabled(enabled);
            }
        }
    }

    // =========================================================================
    // Slots — IpcClient callbacks
    // =========================================================================

    void MainViewModel::onServiceConnected() {
        m_serviceConnected = true;
        m_trayManager->updateState(true, m_driverMode, m_micEnabled ? 1 : 0);
        emit serviceConnectedChanged();

        // Request full status and device list immediately on connect
        m_ipcClient->sendGetStatus();
        m_ipcClient->sendGetDeviceList();
    }

    void MainViewModel::onServiceDisconnected() {
        m_serviceConnected = false;
        m_driverMode = 0;
        m_inputLevel = 0.0f;
        m_outputLevel = 0.0f;
        m_trayManager->updateState(false, 0, 0);
        emit serviceConnectedChanged();
        emit inputLevelChanged();
        emit outputLevelChanged();
        emit telemetryUpdated();
    }

    void MainViewModel::onTelemetryReceived(
        float cpu, float mem, float /*peakMs*/, float p99Ms,
        uint32_t xruns, float /*uptime*/,
        uint8_t drvMode, uint8_t aiEnabled,
        const QString& profile, const QString& model,
        const QString& currentInput, const QString& currentOutput,
        float inputRms, float outputRms, uint8_t monitorEnabled)
    {
        m_cpu        = cpu;
        m_mem        = mem;
        m_p99        = p99Ms;
        m_xruns      = xruns;
        m_driverMode = static_cast<int>(drvMode);

        // Smooth decaying audio VU levels
        m_inputLevel = std::max(inputRms, m_inputLevel * 0.82f);
        m_outputLevel = std::max(outputRms, m_outputLevel * 0.82f);
        emit inputLevelChanged();
        emit outputLevelChanged();

        bool monActive = (monitorEnabled != 0);
        if (m_monitorEnabled != monActive) {
            m_monitorEnabled = monActive;
            emit monitorEnabledChanged();
        }

        if (!profile.isEmpty() && m_activeProfile != profile) {
            m_activeProfile = profile;
            emit activeProfileChanged(m_activeProfile);
        }

        if (!model.isEmpty() && m_currentModel != model) {
            m_currentModel = model;
            emit currentModelChanged();
        }

        if (!currentInput.isEmpty() && m_currentMicrophone != currentInput) {
            m_currentMicrophone = currentInput;
            emit currentMicrophoneChanged();
        }

        if (!currentOutput.isEmpty() && m_currentOutput != currentOutput) {
            m_currentOutput = currentOutput;
            emit currentOutputChanged();
        }

        m_trayManager->updateState(m_serviceConnected, drvMode, aiEnabled);
        emit telemetryUpdated();
    }

    void MainViewModel::onDeviceListReceived(
        const QStringList& inNames, const QStringList& inIds,
        const QStringList& outNames, const QStringList& outIds,
        const QString& selectedInputId, const QString& selectedOutputId)
    {
        m_inputNames = inNames;
        m_inputIds   = inIds;
        m_outputNames = outNames;
        m_outputIds   = outIds;

        int inIdx = m_inputIds.indexOf(selectedInputId);
        m_selectedInputIdx = (inIdx >= 0) ? inIdx : 0;
        if (m_selectedInputIdx < m_inputNames.size()) {
            m_currentMicrophone = m_inputNames.at(m_selectedInputIdx);
            emit currentMicrophoneChanged();
        }

        int outIdx = m_outputIds.indexOf(selectedOutputId);
        m_selectedOutputIdx = (outIdx >= 0) ? outIdx : 0;
        if (m_selectedOutputIdx < m_outputNames.size()) {
            m_currentOutput = m_outputNames.at(m_selectedOutputIdx);
            emit currentOutputChanged();
        }

        emit devicesChanged();
    }

    void MainViewModel::onCommandResponseReceived(uint8_t originalCommand,
                                                   uint8_t status,
                                                   const QString& message)
    {
        const bool ok = (status == 0); // CommandStatus::Success
        emit commandFeedback(message, ok);

        // Refresh tray on state change commands
        if (originalCommand == 0x04 || originalCommand == 0x05) { // Enable/DisableProcessing
            bool nowEnabled = (originalCommand == 0x04);
            m_micEnabled = nowEnabled;
            emit microphoneEnabledChanged();
        }
    }

} // namespace VoiceClear::UI
