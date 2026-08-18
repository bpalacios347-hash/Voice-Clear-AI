#pragma once
/**
 * @file AutoUpdater.h
 * @brief Real HTTP auto-updater using Windows WinHTTP.
 */

#include <string>
#include <functional>

namespace VoiceClear::Utils {

    struct UpdateManifest {
        std::string version;
        std::string releaseNotes;
        std::string downloadUrl;
        std::string sha256;
        std::string minimumSupportedVersion;
    };

    /**
     * @brief Checks for application updates and verifies downloaded installers.
     *
     * Implementation uses WinHTTP (no external dependencies) for HTTPS requests
     * and BCrypt for SHA-256 verification of downloaded files.
     */
    class AutoUpdater {
    public:
        using UpdateCallback = std::function<void(bool updateAvailable, const UpdateManifest& manifest)>;

        /**
         * @brief Asynchronously queries the update manifest JSON from GitHub Pages.
         * Fires callback on a background thread when the result is ready.
         * The manifest URL is: https://voiceclearai.github.io/releases/version.json
         */
        static void CheckForUpdates(const std::string& currentVersion, UpdateCallback callback);

        /**
         * @brief Validates the downloaded installer via BCrypt SHA-256.
         * @return true if the hash matches, false if the file is corrupt or tampered.
         */
        static bool VerifyInstaller(const std::string& installerPath,
                                    const std::string& expectedSha256);

        /**
         * @brief Internal helper: HTTP POST a file to the given endpoint.
         * Exposed for CrashReporter to reuse when uploading .dmp files.
         */
        static bool UploadFile(const std::wstring& host, const std::wstring& path,
                               const std::string& fieldName,
                               const std::string& filePath);
    };

} // namespace VoiceClear::Utils
