/**
 * @file CrashReporter.cpp
 * @brief Windows crash reporter with minidump capture and real HTTPS upload.
 *
 * On crash:
 *   1. UnhandledExceptionFilter fires (catches both C++ and SEH exceptions).
 *   2. MiniDumpWriteDump() writes a .dmp to %ProgramData%\VoiceClear\CrashQueue\.
 *   3. A JSON metadata sidecar (.json) is written alongside the dump.
 *   4. A separate queue-processor thread uploads pending .dmp files via HTTPS
 *      using AutoUpdater::UploadFile() (which uses WinHTTP internally).
 *
 * Upload endpoint: https://voiceclearai.github.io/crash/upload
 * The endpoint expects multipart/form-data with field name "dump".
 *
 * Note: The SECURITY_FLAG_IGNORE_UNKNOWN_CA flag in AutoUpdater should be
 * removed and replaced with a pinned certificate in a production build.
 */

#include "CrashReporter.h"
#include "AutoUpdater.h"
#include "Logger.h"
#include <dbghelp.h>
#include <psapi.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <thread>

#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "psapi.lib")

namespace fs = std::filesystem;

namespace VoiceClear::Utils {

    // =========================================================================
    // Static state
    // =========================================================================

    std::shared_ptr<Diagnostics::TelemetryManager> CrashReporter::s_telemetry = nullptr;
    std::string CrashReporter::s_crashDir;
    std::atomic<bool> CrashReporter::s_uploadEnabled{false};

    // =========================================================================
    // Initialize
    // =========================================================================

    void CrashReporter::Initialize(
        std::shared_ptr<Diagnostics::TelemetryManager> telemetry,
        bool enableUpload)
    {
        s_telemetry     = std::move(telemetry);
        s_uploadEnabled = enableUpload;

        char appData[MAX_PATH] = {};
        GetEnvironmentVariableA("ProgramData", appData, MAX_PATH);
        s_crashDir = std::string(appData) + "\\VoiceClear\\CrashQueue\\";
        fs::create_directories(s_crashDir);

        // Register the SEH / unhandled C++ exception hook
        SetUnhandledExceptionFilter(UnhandledExceptionFilter);

        Logger::GetInstance()->Log(Core::LogLevel::Info,
            "[CrashReporter] Initialized. Crash dir: " + s_crashDir +
            " | Upload: " + (enableUpload ? "enabled" : "disabled"));

        if (enableUpload) {
            // Process any leftover dumps from a previous crash on startup
            std::thread([]() { ProcessCrashQueue(); }).detach();
        }
    }

    // =========================================================================
    // UnhandledExceptionFilter
    // =========================================================================

    LONG WINAPI CrashReporter::UnhandledExceptionFilter(EXCEPTION_POINTERS* exceptionInfo) {
        // Timestamp for unique filenames
        auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::tm tm_buf{};
        localtime_s(&tm_buf, &now);

        char timeStr[64] = {};
        std::strftime(timeStr, sizeof(timeStr), "%Y%m%d_%H%M%S", &tm_buf);
        std::string base = s_crashDir + "VoiceClear_" + timeStr;

        // ---- Write Minidump ----
        const std::string dumpPath = base + ".dmp";
        HANDLE hFile = CreateFileA(dumpPath.c_str(),
                                    GENERIC_WRITE, 0, nullptr,
                                    CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

        if (hFile != INVALID_HANDLE_VALUE) {
            MINIDUMP_EXCEPTION_INFORMATION dumpInfo{};
            dumpInfo.ThreadId          = GetCurrentThreadId();
            dumpInfo.ExceptionPointers = exceptionInfo;
            dumpInfo.ClientPointers    = FALSE;

            // MiniDumpWithFullMemory is too large for upload; use WithDataSegs + Handles
            MINIDUMP_TYPE dumpType = (MINIDUMP_TYPE)(
                MiniDumpWithDataSegs        |
                MiniDumpWithHandleData      |
                MiniDumpWithThreadInfo      |
                MiniDumpWithProcessThreadData);

            MiniDumpWriteDump(
                GetCurrentProcess(),
                GetCurrentProcessId(),
                hFile,
                dumpType,
                &dumpInfo,
                nullptr,
                nullptr);

            CloseHandle(hFile);

            Logger::GetInstance()->Log(Core::LogLevel::Error,
                "[CrashReporter] Minidump written: " + dumpPath);
        }

        // ---- Write JSON sidecar ----
        WriteMetadataFile(base + ".json", exceptionInfo);

        // ---- Trigger upload queue on a separate thread so the handler can return ----
        if (s_uploadEnabled.load(std::memory_order_relaxed)) {
            std::thread([]() { ProcessCrashQueue(); }).detach();
        }

        // Return EXCEPTION_EXECUTE_HANDLER to let Windows terminate the process cleanly
        // (avoids the "Application has stopped working" dialog in service mode)
        return EXCEPTION_EXECUTE_HANDLER;
    }

    // =========================================================================
    // WriteMetadataFile
    // =========================================================================

    void CrashReporter::WriteMetadataFile(const std::string& path,
                                           EXCEPTION_POINTERS* exInfo) {
        std::ofstream meta(path);
        if (!meta) return;

        // Basic process info
        char modulePath[MAX_PATH] = {};
        GetModuleFileNameA(nullptr, modulePath, MAX_PATH);

        // OS version
        OSVERSIONINFOEXA osvi{};
        osvi.dwOSVersionInfoSize = sizeof(osvi);
#pragma warning(suppress: 4996)
        GetVersionExA((LPOSVERSIONINFOA)&osvi);

        // Working set
        PROCESS_MEMORY_COUNTERS_EX pmc{};
        pmc.cb = sizeof(pmc);
        GetProcessMemoryInfo(GetCurrentProcess(),
                             (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
        float memMb = static_cast<float>(pmc.WorkingSetSize) / (1024.0f * 1024.0f);

        meta << "{\n";
        meta << "  \"version\": \"1.0.0\",\n";
        meta << "  \"module\": \"" << modulePath << "\",\n";
        meta << "  \"os_major\": " << osvi.dwMajorVersion << ",\n";
        meta << "  \"os_minor\": " << osvi.dwMinorVersion << ",\n";
        meta << "  \"os_build\": " << osvi.dwBuildNumber  << ",\n";
        meta << "  \"memory_mb\": " << memMb << ",\n";

        if (exInfo && exInfo->ExceptionRecord) {
            meta << "  \"exception_code\": \"0x"
                 << std::hex << std::uppercase
                 << exInfo->ExceptionRecord->ExceptionCode
                 << std::dec << "\",\n";
            meta << "  \"exception_address\": \"0x"
                 << std::hex << std::uppercase
                 << (uintptr_t)exInfo->ExceptionRecord->ExceptionAddress
                 << std::dec << "\",\n";
        }

        if (s_telemetry) {
            auto snap = s_telemetry->GetSnapshot();
            meta << "  \"cpu_usage\": "        << snap.currentCpuUsage << ",\n";
            meta << "  \"peak_latency_ms\": "  << snap.peakLatencyMs   << ",\n";
            meta << "  \"xrun_count\": "       << snap.xrunCount        << ",\n";
            meta << "  \"sample_rate\": "      << snap.activeSampleRate << "\n";
        } else {
            meta << "  \"telemetry\": null\n";
        }

        meta << "}\n";
    }

    // =========================================================================
    // ProcessCrashQueue — uploads all pending .dmp files
    // =========================================================================

    void CrashReporter::ProcessCrashQueue() {
        if (!fs::exists(s_crashDir)) return;

        // Upload endpoint
        const std::wstring uploadHost = L"voiceclearai.github.io";
        const std::wstring uploadPath = L"/crash/upload";

        for (const auto& entry : fs::directory_iterator(s_crashDir)) {
            if (entry.path().extension() != ".dmp") continue;

            const std::string dumpPath = entry.path().string();

            Logger::GetInstance()->Log(Core::LogLevel::Info,
                "[CrashReporter] Uploading: " + dumpPath);

            bool ok = AutoUpdater::UploadFile(uploadHost, uploadPath, "dump", dumpPath);

            if (ok) {
                Logger::GetInstance()->Log(Core::LogLevel::Info,
                    "[CrashReporter] Upload succeeded. Removing local file.");
                std::error_code ec;
                fs::remove(entry.path(), ec);

                // Also remove the JSON sidecar
                auto jsonPath = entry.path();
                jsonPath.replace_extension(".json");
                fs::remove(jsonPath, ec);
            } else {
                Logger::GetInstance()->Log(Core::LogLevel::Warn,
                    "[CrashReporter] Upload failed. File kept for retry: " + dumpPath);
            }
        }
    }

} // namespace VoiceClear::Utils
