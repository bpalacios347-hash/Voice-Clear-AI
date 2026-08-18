add_library(VoiceClearUtils STATIC
    Logger.cpp
    CrashReporter.cpp
    AutoUpdater.cpp
)

target_link_libraries(VoiceClearUtils PUBLIC
    CoreInterfaces
    nlohmann_json::nlohmann_json
    Dbghelp       # MiniDumpWriteDump
    Psapi         # GetProcessMemoryInfo
    winhttp       # WinHTTP for AutoUpdater + CrashReporter upload
    bcrypt        # SHA-256 for AutoUpdater::VerifyInstaller
)

target_include_directories(VoiceClearUtils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
