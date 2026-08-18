#include <catch2/catch_test_macros.hpp>
#include <windows.h>
#include <thread>
#include "../src/diagnostics/IpcServer.h"
#include "../src/core/ipc/IpcProtocol.h"

using namespace VoiceClear::Diagnostics;

TEST_CASE("IPC Server Handshake and Telemetry Streaming", "[IpcServer]") {
    // Scaffold an IPC server locally for the test
    auto telemetry = std::make_shared<VoiceClear::Diagnostics::TelemetryManager>();
    IpcServer server(telemetry, nullptr);
    server.Start();

    // Give it a moment to spawn the Named Pipe
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Connect as client
    HANDLE hPipe = CreateFileA(
        VoiceClear::IPC::PIPE_NAME,
        GENERIC_READ | GENERIC_WRITE,
        0, nullptr, OPEN_EXISTING, 0, nullptr
    );

    REQUIRE(hPipe != INVALID_HANDLE_VALUE);

    // Send Handshake
    VoiceClear::IPC::HandshakeRequest req{};
    req.protocolVersion = VoiceClear::IPC::PROTOCOL_VERSION;
    req.clientType = VoiceClear::IPC::ClientType::GUI;

    VoiceClear::IPC::MessageHeader outHeader = VoiceClear::IPC::MakeHeader(
        VoiceClear::IPC::MessageType::Handshake, sizeof(req));

    DWORD bytesWritten = 0;
    WriteFile(hPipe, &outHeader, sizeof(outHeader), &bytesWritten, nullptr);
    WriteFile(hPipe, &req, sizeof(req), &bytesWritten, nullptr);

    // Read Response
    VoiceClear::IPC::MessageHeader inHeader{};
    DWORD bytesRead = 0;
    REQUIRE(ReadFile(hPipe, &inHeader, sizeof(inHeader), &bytesRead, nullptr));
    REQUIRE(inHeader.type == VoiceClear::IPC::MessageType::Handshake);

    VoiceClear::IPC::HandshakeResponse res{};
    REQUIRE(ReadFile(hPipe, &res, sizeof(res), &bytesRead, nullptr));
    REQUIRE(res.accepted == 1);

    // Wait for telemetry payload
    REQUIRE(ReadFile(hPipe, &inHeader, sizeof(inHeader), &bytesRead, nullptr));
    REQUIRE(inHeader.type == VoiceClear::IPC::MessageType::TelemetryBinary);
    REQUIRE(inHeader.payloadSize == sizeof(VoiceClear::IPC::TelemetryPayload));

    VoiceClear::IPC::TelemetryPayload payload{};
    REQUIRE(ReadFile(hPipe, &payload, sizeof(payload), &bytesRead, nullptr));
    REQUIRE(payload.memoryUsageMb >= 0.0f);

    CloseHandle(hPipe);
    server.Stop();
}
