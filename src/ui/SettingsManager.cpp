#include "SettingsManager.h"
#include <QFile>
#include <QDir>
#include <QStandardPaths>

namespace VoiceClear::UI {

    SettingsManager::SettingsManager(QObject* parent) : QObject(parent) {
        QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir dir(appData);
        if (!dir.exists()) dir.mkpath(".");
        m_settingsPath = dir.filePath("settings.json");
        load();
    }

    void SettingsManager::load() {
        QFile file(m_settingsPath);
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray data = file.readAll();
            QJsonDocument doc = QJsonDocument::fromJson(data);
            QJsonObject obj = doc.object();

            if (obj.contains("microphone")) m_selectedMicrophone = obj["microphone"].toString();
            if (obj.contains("model")) m_selectedModel = obj["model"].toString();
            if (obj.contains("theme")) m_theme = obj["theme"].toString();
            if (obj.contains("startMinimized")) m_startMinimized = obj["startMinimized"].toBool();
            if (obj.contains("diagnosticsVisible")) m_diagnosticsVisible = obj["diagnosticsVisible"].toBool();
        }
    }

    void SettingsManager::save() {
        QJsonObject obj;
        obj["microphone"] = m_selectedMicrophone;
        obj["model"] = m_selectedModel;
        obj["theme"] = m_theme;
        obj["startMinimized"] = m_startMinimized;
        obj["diagnosticsVisible"] = m_diagnosticsVisible;

        QJsonDocument doc(obj);
        QFile file(m_settingsPath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(doc.toJson());
        }
    }

    QString SettingsManager::selectedMicrophone() const { return m_selectedMicrophone; }
    void SettingsManager::setSelectedMicrophone(const QString& val) {
        if (m_selectedMicrophone != val) {
            m_selectedMicrophone = val;
            emit selectedMicrophoneChanged();
            save();
        }
    }

    QString SettingsManager::selectedModel() const { return m_selectedModel; }
    void SettingsManager::setSelectedModel(const QString& val) {
        if (m_selectedModel != val) {
            m_selectedModel = val;
            emit selectedModelChanged();
            save();
        }
    }

    QString SettingsManager::theme() const { return m_theme; }
    void SettingsManager::setTheme(const QString& val) {
        if (m_theme != val) {
            m_theme = val;
            emit themeChanged();
            save();
        }
    }

    bool SettingsManager::startMinimized() const { return m_startMinimized; }
    void SettingsManager::setStartMinimized(bool val) {
        if (m_startMinimized != val) {
            m_startMinimized = val;
            emit startMinimizedChanged();
            save();
        }
    }

    bool SettingsManager::diagnosticsVisible() const { return m_diagnosticsVisible; }
    void SettingsManager::setDiagnosticsVisible(bool val) {
        if (m_diagnosticsVisible != val) {
            m_diagnosticsVisible = val;
            emit diagnosticsVisibleChanged();
            save();
        }
    }
}
