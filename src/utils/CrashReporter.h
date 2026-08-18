#pragma once
/**
 * @file CrashReporter.h
 * @brief Windows crash reporter: minidump + JSON sidecar + real HTTPS upload.
 */

#include <windows.h>
#include <string>
#include <memory>
#include <atomic>
#include "../diagnostics/TelemetryManager.h"

namespace VoiceClear::Utils {

    /**
     * @brief Catches unhandled C++ and SEH exceptions via SetUnhandledExceptionFilter.
     *
     * On crash:
     *   - Writes a MiniDump (.dmp) to %ProgramData%\VoiceClear\CrashQueue\
     *   - Writes a JSON sidecar with OS version, exception code, telemetry snapshot
     *   - Queues the dump for async HTTPS upload via AutoUpdater::UploadFile()
     *   - Retains failed uploads for retry on the next service start
     */
    class CrashReporter {
    public:
        /**
         * @brief Must be called once at service startup (before any threads are created).
         * @param telemetry   Optional telemetry snapshot to embed in crash metadata.
         * @param enableUpload If true, queued dumps are uploaded on a background thread.
         */
        static void Initialize(
            std::shared_ptr<Diagnostics::TelemetryManager> telemetry,
            bool enableUpload = true);

    private:
        static LONG WINAPI UnhandledExceptionFilter(EXCEPTION_POINTERS* exceptionInfo);
        static void WriteMetadataFile(const std::string& path,
                                      EXCEPTION_POINTERS* exInfo = nullptr);
        static void ProcessCrashQueue();

        static std::shared_ptr<Diagnostics::TelemetryManager> s_telemetry;
        static std::string                                     s_crashDir;
        static std::atomic<bool>                               s_uploadEnabled;
    };

} // namespace VoiceClear::Utils
