add_library(VoiceClearDiagnostics STATIC
    IpcServer.cpp
    TelemetryManager.cpp
    LockFreeTelemetryQueue.h   # Header-only
)

target_include_directories(VoiceClearDiagnostics PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${ONNXRUNTIME_INCLUDE_DIR}
)

target_link_libraries(VoiceClearDiagnostics PUBLIC
    VoiceClearCore     # IpcProtocol.h + core interfaces
    VoiceClearUtils    # Logger, CrashReporter
    spdlog::spdlog
    Psapi              # GetProcessMemoryInfo in IpcServer::BuildTelemetry
    advapi32           # ConvertStringSecurityDescriptorToSecurityDescriptor (DACL)
)
