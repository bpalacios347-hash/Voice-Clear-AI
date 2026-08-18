#include <spdlog/spdlog.h>
#include <iostream>
#include <thread>
#include <chrono>
#include "core/ipc/IpcServer.h"
#include "utils/Logger.h"

// Skeleton for the Windows Service entry point
int main()
{
    auto logger = VoiceClear::Utils::Logger::GetInstance();
    logger->Log(VoiceClear::Core::LogLevel::Info, "Voice Clear AI - Audio Service initializing.");

    auto ipcServer = VoiceClear::Core::IPC::CreateIpcServer();
    ipcServer->SetMessageHandler([&logger](const std::string& msg) {
        logger->Log(VoiceClear::Core::LogLevel::Debug, "Received from UI: " + msg);
    });

    if (ipcServer->Start("VoiceClearIPC")) {
        // Send heartbeat
        for (int i=0; i<3; ++i) {
            ipcServer->SendMessageToClient("Heartbeat from Service");
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        ipcServer->Stop();
    }
    
    return 0;
}
