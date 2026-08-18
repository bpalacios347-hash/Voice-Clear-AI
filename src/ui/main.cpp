#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include "MainViewModel.h"

void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    QFile outFile("logs/ui_debug.log");
    if (outFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
        QTextStream ts(&outFile);
        QString typeStr;
        switch (type) {
            case QtDebugMsg: typeStr = "DEBUG"; break;
            case QtInfoMsg: typeStr = "INFO"; break;
            case QtWarningMsg: typeStr = "WARN"; break;
            case QtCriticalMsg: typeStr = "CRITICAL"; break;
            case QtFatalMsg: typeStr = "FATAL"; break;
        }
        ts << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz")
           << " [" << typeStr << "] " << msg << " (" << context.file << ":" << context.line << ")\n";
    }
}

#include <windows.h>
#include <filesystem>

int main(int argc, char *argv[])
{
    char exePath[MAX_PATH] = {};
    if (GetModuleFileNameA(nullptr, exePath, MAX_PATH) > 0) {
        std::filesystem::path dir = std::filesystem::path(exePath).parent_path();
        SetCurrentDirectoryA(dir.string().c_str());
    }

    qInstallMessageHandler(customMessageHandler);

    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);

    QQmlApplicationEngine engine;

    // Register ViewModel
    VoiceClear::UI::MainViewModel mainViewModel;
    engine.rootContext()->setContextProperty("viewModel", &mainViewModel);

    const QUrl url(u"qrc:/qml/main.qml"_qs);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
