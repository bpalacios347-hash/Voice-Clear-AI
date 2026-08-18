#include "SystemTrayManager.h"
#include <QAction>
#include <QApplication>
#include <QIcon>
#include <QStyle>

namespace VoiceClear::UI {

    SystemTrayManager::SystemTrayManager(QObject* parent) 
        : QObject(parent), 
          m_trayIcon(new QSystemTrayIcon(this)), 
          m_trayMenu(new QMenu()) 
    {
        createActions();

        // Standard Qt icon for tray
        QIcon icon = QApplication::style()->standardIcon(QStyle::SP_MediaVolume);
        m_trayIcon->setIcon(icon);
        m_trayIcon->setContextMenu(m_trayMenu);
        m_trayIcon->show();

        connect(m_trayIcon, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
            if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick) {
                emit toggleWindowRequested();
            }
        });

        // Initialize state as offline
        updateState(false, 0, 0);
    }

    SystemTrayManager::~SystemTrayManager() = default;

    void SystemTrayManager::createActions() {
        m_statusAction = new QAction("Status: Offline", this);
        m_statusAction->setEnabled(false);
        m_trayMenu->addAction(m_statusAction);

        m_driverAction = new QAction("Driver: Unknown", this);
        m_driverAction->setEnabled(false);
        m_trayMenu->addAction(m_driverAction);
        
        m_trayMenu->addSeparator();

        m_enableAction = new QAction("Enable Processing", this);
        connect(m_enableAction, &QAction::triggered, this, [this]() { emit enableProcessing(true); });
        m_trayMenu->addAction(m_enableAction);

        m_disableAction = new QAction("Disable Processing", this);
        connect(m_disableAction, &QAction::triggered, this, [this]() { emit enableProcessing(false); });
        m_trayMenu->addAction(m_disableAction);

        m_trayMenu->addSeparator();

        QMenu* profileMenu = m_trayMenu->addMenu("Select Profile");
        
        m_profBalanced = new QAction("Balanced", this);
        connect(m_profBalanced, &QAction::triggered, this, [this]() { emit profileSelected("Balanced"); });
        profileMenu->addAction(m_profBalanced);

        m_profStrong = new QAction("Strong Noise Removal", this);
        connect(m_profStrong, &QAction::triggered, this, [this]() { emit profileSelected("Strong"); });
        profileMenu->addAction(m_profStrong);

        m_profVoicePreservation = new QAction("Voice Preservation", this);
        connect(m_profVoicePreservation, &QAction::triggered, this, [this]() { emit profileSelected("Voice Preservation"); });
        profileMenu->addAction(m_profVoicePreservation);

        m_profLowCpu = new QAction("Low CPU", this);
        connect(m_profLowCpu, &QAction::triggered, this, [this]() { emit profileSelected("Low CPU"); });
        profileMenu->addAction(m_profLowCpu);

        m_trayMenu->addSeparator();

        QAction* showAction = new QAction("Show Diagnostics UI", this);
        connect(showAction, &QAction::triggered, this, &SystemTrayManager::toggleWindowRequested);
        m_trayMenu->addAction(showAction);

        QAction* quitAction = new QAction("Quit UI", this);
        connect(quitAction, &QAction::triggered, this, &SystemTrayManager::quitRequested);
        m_trayMenu->addAction(quitAction);
    }

    void SystemTrayManager::updateState(bool connected, uint8_t driverMode, uint8_t aiEnabled) {
        if (!connected) {
            m_statusAction->setText("Service: Offline");
            m_driverAction->setText("Driver: Unknown");
            m_enableAction->setEnabled(false);
            m_disableAction->setEnabled(false);
            m_profBalanced->setEnabled(false);
            m_profStrong->setEnabled(false);
            m_profVoicePreservation->setEnabled(false);
            m_profLowCpu->setEnabled(false);
            return;
        }

        m_statusAction->setText("Service: Running");
        
        if (driverMode == 1) {
            m_driverAction->setText("Driver: Real (AVStream)");
        } else if (driverMode == 2) {
            m_driverAction->setText("Driver: Mock (/dev/null)");
        } else {
            m_driverAction->setText("Driver: Missing");
        }

        bool enabled = (aiEnabled != 0);
        m_enableAction->setEnabled(!enabled);
        m_disableAction->setEnabled(enabled);

        m_profBalanced->setEnabled(true);
        m_profStrong->setEnabled(true);
        m_profVoicePreservation->setEnabled(true);
        m_profLowCpu->setEnabled(true);
    }

}
