#pragma once
#include <QObject>
#include <QSystemTrayIcon>
#include <QMenu>

namespace VoiceClear::UI {

    class SystemTrayManager : public QObject {
        Q_OBJECT
    public:
        explicit SystemTrayManager(QObject* parent = nullptr);
        ~SystemTrayManager() override;

    public slots:
        void updateState(bool connected, uint8_t driverMode, uint8_t aiEnabled);

    signals:
        void profileSelected(const QString& profileId);
        void enableProcessing(bool enable);
        void toggleWindowRequested();
        void quitRequested();

    private:
        QSystemTrayIcon* m_trayIcon;
        QMenu* m_trayMenu;
        QAction* m_statusAction;
        QAction* m_driverAction;
        
        QAction* m_enableAction;
        QAction* m_disableAction;

        QAction* m_profBalanced;
        QAction* m_profStrong;
        QAction* m_profVoicePreservation;
        QAction* m_profLowCpu;

        void createActions();
        void updateMenuState(bool connected, uint8_t driverMode, uint8_t aiEnabled);
    };

}
