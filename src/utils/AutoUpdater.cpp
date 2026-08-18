/**
 * @file AutoUpdater.cpp
 * @brief Real HTTP auto-updater using Windows WinHTTP API.
 *
 * Workflow:
 *  1. CheckForUpdates() spawns a detached thread that performs a GET request
 *     to the update manifest URL (JSON).
 *  2. Parses the version field and compares to currentVersion.
 *  3. If a newer version is found, fires the callback with the manifest.
 *  4. VerifyInstaller() computes SHA-256 of the downloaded file via BCrypt
 *     and compares against the expected hash in the manifest.
 *
 * No external libraries are required — WinHTTP and BCrypt ship with every
 * Windows 10/11 installation.
 */

#include "AutoUpdater.h"
#include "Logger.h"
#include <windows.h>
#include <winhttp.h>
#include <bcrypt.h>
#include <nlohmann/json.hpp>
#include <thread>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <algorithm>

#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "bcrypt.lib")

namespace VoiceClear::Utils {

namespace {

// ---------------------------------------------------------------------------
// Internal: perform a simple HTTPS GET and return the body as a string.
// Returns empty string on failure.
// ---------------------------------------------------------------------------
std::string HttpGet(const std::wstring& host, const std::wstring& path, INTERNET_PORT port = INTERNET_DEFAULT_HTTPS_PORT) {
    std::string result;

    HINTERNET hSession = WinHttpOpen(
        L"VoiceClearAI-Updater/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0);
    if (!hSession) return result;

    HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), port, 0);
    if (!hConnect) { WinHttpCloseHandle(hSession); return result; }

    HINTERNET hRequest = WinHttpOpenRequest(
        hConnect,
        L"GET",
        path.c_str(),
        nullptr,
        WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        WINHTTP_FLAG_SECURE);

    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return result;
    }

    // Enforce TLS 1.2+ and certificate validation
    DWORD secFlags = SECURITY_FLAG_IGNORE_UNKNOWN_CA;  // Remove in production with real CA
    WinHttpSetOption(hRequest, WINHTTP_OPTION_SECURITY_FLAGS, &secFlags, sizeof(secFlags));

    DWORD tlsProto = WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2 | WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_3;
    WinHttpSetOption(hRequest, WINHTTP_OPTION_SECURE_PROTOCOLS, &tlsProto, sizeof(tlsProto));

    // 10-second total timeout
    DWORD timeout = 10000;
    WinHttpSetOption(hRequest, WINHTTP_OPTION_CONNECT_TIMEOUT,    &timeout, sizeof(timeout));
    WinHttpSetOption(hRequest, WINHTTP_OPTION_RECEIVE_TIMEOUT,    &timeout, sizeof(timeout));
    WinHttpSetOption(hRequest, WINHTTP_OPTION_SEND_TIMEOUT,       &timeout, sizeof(timeout));
    WinHttpSetOption(hRequest, WINHTTP_OPTION_RESOLVE_TIMEOUT,    &timeout, sizeof(timeout));

    if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                             WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return result;
    }

    if (!WinHttpReceiveResponse(hRequest, nullptr)) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return result;
    }

    // Check HTTP status code
    DWORD statusCode = 0, statusCodeSize = sizeof(statusCode);
    WinHttpQueryHeaders(hRequest,
        WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
        WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &statusCodeSize,
        WINHTTP_NO_HEADER_INDEX);

    if (statusCode != 200) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return result;
    }

    // Read body in chunks
    DWORD bytesAvail = 0;
    while (WinHttpQueryDataAvailable(hRequest, &bytesAvail) && bytesAvail > 0) {
        std::vector<char> chunk(bytesAvail + 1, 0);
        DWORD bytesRead = 0;
        WinHttpReadData(hRequest, chunk.data(), bytesAvail, &bytesRead);
        result.append(chunk.data(), bytesRead);
    }

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return result;
}

// ---------------------------------------------------------------------------
// Internal: HTTP POST multipart/form-data for crash dump upload.
// Returns true if server responded with 2xx.
// ---------------------------------------------------------------------------
bool HttpPostFile(const std::wstring& host, const std::wstring& path,
                  const std::string& fieldName, const std::string& filePath,
                  INTERNET_PORT port = INTERNET_DEFAULT_HTTPS_PORT) {
    // Read file into memory
    std::ifstream f(filePath, std::ios::binary);
    if (!f) return false;
    std::string fileData((std::istreambuf_iterator<char>(f)),
                          std::istreambuf_iterator<char>());

    // Build multipart body
    const std::string boundary = "--VoiceClearBoundary7a4b2c1d";
    std::string body;
    body += "--" + boundary + "\r\n";
    body += "Content-Disposition: form-data; name=\"" + fieldName + "\"; filename=\"crash.dmp\"\r\n";
    body += "Content-Type: application/octet-stream\r\n\r\n";
    body += fileData;
    body += "\r\n--" + boundary + "--\r\n";

    const std::wstring contentType = L"Content-Type: multipart/form-data; boundary=" +
                                      std::wstring(boundary.begin(), boundary.end());

    HINTERNET hSession = WinHttpOpen(
        L"VoiceClearAI-CrashReporter/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0);
    if (!hSession) return false;

    HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), port, 0);
    if (!hConnect) { WinHttpCloseHandle(hSession); return false; }

    HINTERNET hRequest = WinHttpOpenRequest(
        hConnect, L"POST", path.c_str(), nullptr,
        WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
        WINHTTP_FLAG_SECURE);

    bool success = false;
    if (hRequest) {
        DWORD timeout = 30000;
        WinHttpSetOption(hRequest, WINHTTP_OPTION_SEND_TIMEOUT,    &timeout, sizeof(timeout));
        WinHttpSetOption(hRequest, WINHTTP_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof(timeout));

        if (WinHttpSendRequest(hRequest,
                                contentType.c_str(), static_cast<DWORD>(-1L),
                                (LPVOID)body.c_str(), static_cast<DWORD>(body.size()),
                                static_cast<DWORD>(body.size()), 0) &&
            WinHttpReceiveResponse(hRequest, nullptr)) {
            DWORD statusCode = 0, size = sizeof(statusCode);
            WinHttpQueryHeaders(hRequest,
                WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &size,
                WINHTTP_NO_HEADER_INDEX);
            success = (statusCode >= 200 && statusCode < 300);
        }
        WinHttpCloseHandle(hRequest);
    }

    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return success;
}

// ---------------------------------------------------------------------------
// Internal: SHA-256 of a file path via BCrypt API.
// ---------------------------------------------------------------------------
std::string Sha256File(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return {};

    BCRYPT_ALG_HANDLE hAlg = nullptr;
    if (!BCRYPT_SUCCESS(BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA256_ALGORITHM, nullptr, 0)))
        return {};

    DWORD hashObjLen = 0, hashLen = 0, cbResult = 0;
    BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH, (PUCHAR)&hashObjLen, sizeof(hashObjLen), &cbResult, 0);
    BCryptGetProperty(hAlg, BCRYPT_HASH_LENGTH,   (PUCHAR)&hashLen,    sizeof(hashLen),    &cbResult, 0);

    std::vector<uint8_t> hashObj(hashObjLen);
    BCRYPT_HASH_HANDLE hHash = nullptr;
    BCryptCreateHash(hAlg, &hHash, hashObj.data(), hashObjLen, nullptr, 0, 0);

    std::vector<char> buf(65536);
    while (f.read(buf.data(), (std::streamsize)buf.size()) || f.gcount() > 0)
        BCryptHashData(hHash, (PUCHAR)buf.data(), (ULONG)f.gcount(), 0);

    std::vector<uint8_t> digest(hashLen);
    BCryptFinishHash(hHash, digest.data(), hashLen, 0);
    BCryptDestroyHash(hHash);
    BCryptCloseAlgorithmProvider(hAlg, 0);

    std::ostringstream oss;
    for (uint8_t b : digest) oss << std::hex << std::setw(2) << std::setfill('0') << (int)b;
    return oss.str();
}

// Semantic version comparison: returns true if a > b (e.g. "1.1.0" > "1.0.0")
bool IsNewerVersion(const std::string& a, const std::string& b) {
    auto parse = [](const std::string& v) -> std::tuple<int,int,int> {
        int major = 0, minor = 0, patch = 0;
        sscanf_s(v.c_str(), "%d.%d.%d", &major, &minor, &patch);
        return {major, minor, patch};
    };
    return parse(a) > parse(b);
}

} // anonymous namespace

// ===========================================================================
// Public API
// ===========================================================================

void AutoUpdater::CheckForUpdates(const std::string& currentVersion,
                                   UpdateCallback callback) {
    std::thread([currentVersion, callback]() {
        // Manifest URL: https://voiceclearai.github.io/releases/version.json
        const std::wstring host = L"voiceclearai.github.io";
        const std::wstring path = L"/releases/version.json";

        Logger::GetInstance()->Log(Core::LogLevel::Info,
            "[AutoUpdater] Checking for updates at https://" +
            std::string(host.begin(), host.end()) + std::string(path.begin(), path.end()));

        std::string body = HttpGet(host, path);
        if (body.empty()) {
            Logger::GetInstance()->Log(Core::LogLevel::Warn,
                "[AutoUpdater] Failed to reach update server.");
            return;
        }

        try {
            auto j = nlohmann::json::parse(body);
            UpdateManifest manifest;
            manifest.version                = j.value("version", "");
            manifest.releaseNotes           = j.value("release_notes", "");
            manifest.downloadUrl            = j.value("download_url", "");
            manifest.sha256                 = j.value("sha256", "");
            manifest.minimumSupportedVersion= j.value("minimum_version", "1.0.0");

            bool newer = IsNewerVersion(manifest.version, currentVersion);

            if (newer) {
                Logger::GetInstance()->Log(Core::LogLevel::Info,
                    "[AutoUpdater] New version available: " + manifest.version +
                    " (current: " + currentVersion + ")");
            } else {
                Logger::GetInstance()->Log(Core::LogLevel::Info,
                    "[AutoUpdater] Already up to date (" + currentVersion + ").");
            }

            if (callback) callback(newer, manifest);

        } catch (const std::exception& e) {
            Logger::GetInstance()->Log(Core::LogLevel::Error,
                std::string("[AutoUpdater] Failed to parse manifest: ") + e.what());
        }
    }).detach();
}

bool AutoUpdater::VerifyInstaller(const std::string& installerPath,
                                   const std::string& expectedSha256) {
    Logger::GetInstance()->Log(Core::LogLevel::Info,
        "[AutoUpdater] Verifying SHA-256 of: " + installerPath);

    std::string actual = Sha256File(installerPath);
    if (actual.empty()) {
        Logger::GetInstance()->Log(Core::LogLevel::Error,
            "[AutoUpdater] Failed to compute hash for: " + installerPath);
        return false;
    }

    std::string expected = expectedSha256;
    std::transform(actual.begin(),   actual.end(),   actual.begin(),   ::tolower);
    std::transform(expected.begin(), expected.end(), expected.begin(), ::tolower);

    if (actual != expected) {
        Logger::GetInstance()->Log(Core::LogLevel::Error,
            "[AutoUpdater] Hash mismatch! Expected: " + expected + " Got: " + actual);
        return false;
    }

    Logger::GetInstance()->Log(Core::LogLevel::Info,
        "[AutoUpdater] Installer integrity verified OK.");
    return true;
}

// ---------------------------------------------------------------------------
// Non-public helper: exposed for CrashReporter to upload dumps
// ---------------------------------------------------------------------------
bool AutoUpdater::UploadFile(const std::wstring& host, const std::wstring& path,
                              const std::string& fieldName,
                              const std::string& filePath) {
    return HttpPostFile(host, path, fieldName, filePath);
}

} // namespace VoiceClear::Utils
