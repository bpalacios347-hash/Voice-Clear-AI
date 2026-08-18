#pragma once
#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QJsonDocument>

namespace VoiceClear::UI {

    class SettingsManager : public QObject {
        Q_OBJECT
        Q_PROPERTY(QString selectedMicrophone READ selectedMicrophone WRITE setSelectedMicrophone NOTIFY selectedMicrophoneChanged)
        Q_PROPERTY(QString selectedModel READ selectedModel WRITE setSelectedModel NOTIFY selectedModelChanged)
        Q_PROPERTY(QString theme READ theme WRITE setTheme NOTIFY themeChanged)
        Q_PROPERTY(bool startMinimized READ startMinimized WRITE setStartMinimized NOTIFY startMinimizedChanged)
        Q_PROPERTY(bool diagnosticsVisible READ diagnosticsVisible WRITE setDiagnosticsVisible NOTIFY diagnosticsVisibleChanged)

    public:
        explicit SettingsManager(QObject* parent = nullptr);
        ~SettingsManager() override = default;

        void load();
        void save();

        QString selectedMicrophone() const;
        void setSelectedMicrophone(const QString& val);

        QString selectedModel() const;
        void setSelectedModel(const QString& val);

        QString theme() const;
        void setTheme(const QString& val);

        bool startMinimized() const;
        void setStartMinimized(bool val);

        bool diagnosticsVisible() const;
        void setDiagnosticsVisible(bool val);

    signals:
        void selectedMicrophoneChanged();
        void selectedModelChanged();
        void themeChanged();
        void startMinimizedChanged();
        void diagnosticsVisibleChanged();

    private:
        QString m_settingsPath;
        
        QString m_selectedMicrophone{"Default"};
        QString m_selectedModel{"DeepFilterNet3"};
        QString m_theme{"Dark"};
        bool m_startMinimized{false};
        bool m_diagnosticsVisible{false};
    };

}
