#pragma once
/**
 * @file IpcProtocol.h
 * @brief Canonical binary IPC protocol shared between VoiceClearService and VoiceClear.exe.
 *
 * Both the IpcServer (service side) and the IpcClient (UI side) MUST include only this
 * header for protocol definitions.  Any protocol change must be reflected here and the
 * IPC_PROTOCOL_VERSION constant must be bumped.
 *
 * Wire layout per message:
 *   [IpcMessageHeader (9 bytes)] [Payload (header.payloadSize bytes)]
 *
 * All structs are packed (pragma pack 1) to guarantee identical binary layout on all
 * x64 MSVC targets.
 */

#include <cstdint>
#include <cstring>

namespace VoiceClear::IPC {

    // -------------------------------------------------------------------------
    // Constants
    // -------------------------------------------------------------------------

    /// Increment whenever the wire format changes in a backward-incompatible way.
    constexpr uint32_t PROTOCOL_VERSION = 4;

    /// Named-pipe endpoint shared by both sides.
    constexpr const char* PIPE_NAME = "\\\\.\\pipe\\VoiceClearIPC";

    /// Maximum number of simultaneous UI clients (only 1 is currently supported).
    constexpr uint32_t MAX_PIPE_INSTANCES = 1;

    /// Telemetry broadcast rate (ms per frame).
    constexpr uint32_t TELEMETRY_INTERVAL_MS = 50; // 20 Hz

    // -------------------------------------------------------------------------
    // Message Types
    // -------------------------------------------------------------------------

    enum class MessageType : uint8_t {
        Handshake           = 0x01,
        TelemetryBinary     = 0x02,
        SetProfile          = 0x03,
        EnableProcessing    = 0x04,
        DisableProcessing   = 0x05,
        ReloadModel         = 0x06,
        GetStatus           = 0x07,
        CommandResponse     = 0x08,
        Alert               = 0x09,
        StatusResponse      = 0x0A,
        GetDeviceList       = 0x0B,
        DeviceListResponse  = 0x0C,
        SetDevices          = 0x0D,
        SetMonitor          = 0x0E,
        ShutdownUI          = 0x0F,
    };

    // -------------------------------------------------------------------------
    // Command Status Codes
    // -------------------------------------------------------------------------

    enum class CommandStatus : uint8_t {
        Success             = 0x00,
        Failed              = 0x01,
        InvalidProfile      = 0x02,
        ModelLoadError      = 0x03,
        VersionMismatch     = 0x04,
        UnknownCommand      = 0x05,
        InvalidDevice       = 0x06,
    };

    // -------------------------------------------------------------------------
    // Client Type (used in handshake)
    // -------------------------------------------------------------------------

    enum class ClientType : uint32_t {
        Unknown = 0,
        CLI     = 1,
        GUI     = 2,
    };

    // -------------------------------------------------------------------------
    // Wire Structs — ALL packed to 1-byte alignment
    // -------------------------------------------------------------------------

    #pragma pack(push, 1)

    /**
     * @brief Fixed-size header prepended to every message.
     * Total: 4 + 1 + 4 = 9 bytes.
     */
    struct MessageHeader {
        uint32_t    protocolVersion;    ///< Must equal PROTOCOL_VERSION
        MessageType type;               ///< Message type tag
        uint32_t    payloadSize;        ///< Byte count of the following payload (may be 0)
    };
    static_assert(sizeof(MessageHeader) == 9, "MessageHeader size mismatch");

    // --- Handshake ---

    struct HandshakeRequest {
        uint32_t    protocolVersion;
        ClientType  clientType;
    };

    struct HandshakeResponse {
        uint32_t    protocolVersion;
        uint8_t     accepted;           ///< 1 = accepted, 0 = rejected
    };

    // --- Commands ---

    struct SetProfilePayload {
        uint32_t    protocolVersion;
        char        profileId[32];      ///< Null-terminated profile name
        char        modelId[64];        ///< Optional model override; empty = use default
        uint64_t    timestamp;          ///< GetTickCount64() from the sender
    };

    struct ReloadModelPayload {
        char        modelName[128];     ///< Null-terminated model filename (e.g. "deepfilternet3.onnx")
    };

    struct SetMonitorPayload {
        uint8_t     enabled;            ///< 1 = active, 0 = inactive
    };

    // --- Device Selection Structs ---

    struct AudioDeviceEntry {
        char    id[512];
        char    friendlyName[256];
        uint8_t isCapture;              ///< 1 = Input (Mic), 0 = Output (Render/Cable)
        uint8_t isDefault;
        uint8_t isSelected;
    };

    struct DeviceListPayload {
        uint32_t         deviceCount;
        AudioDeviceEntry devices[32];
        char             selectedInputId[512];
        char             selectedOutputId[512];
    };

    struct SetDevicesPayload {
        char inputId[512];
        char outputId[512];
    };

    // --- Responses ---

    struct CommandResponsePayload {
        MessageType originalCommand;    ///< Echo of the command being acknowledged
        CommandStatus status;
        char        message[128];       ///< Optional human-readable detail
    };

    // --- Telemetry ---

    struct TelemetryPayload {
        float       cpuUsage;           ///< Inference CPU % (0-100)
        float       memoryUsageMb;      ///< Process working set in MB
        float       peakLatencyMs;      ///< Peak per-frame inference latency
        float       p99LatencyMs;       ///< P99 per-frame inference latency
        uint32_t    xrunCount;          ///< Cumulative XRUN (dropout) counter
        float       uptimeSeconds;      ///< Service uptime in seconds
        uint8_t     driverMode;         ///< 0 = Offline, 1 = AVStream, 2 = Mock, 3 = WASAPI
        uint8_t     aiEnabled;          ///< 1 = Active, 0 = Bypassed
        char        activeProfile[32];  ///< e.g. "Balanced"
        char        activeModel[128];   ///< e.g. "deepfilternet3.onnx"
        char        currentInputName[256];  ///< Active input device friendly name
        char        currentOutputName[256]; ///< Active output device friendly name
        float       inputRmsLevel;      ///< Raw input VU level (0.0 to 1.0)
        float       outputRmsLevel;     ///< Clean output VU level (0.0 to 1.0)
        uint8_t     monitorEnabled;     ///< 1 = Audio monitor (hear myself) active
    };

    struct StatusResponsePayload {
        TelemetryPayload telemetry;
        uint8_t          serviceState;
    };

    #pragma pack(pop)

    // -------------------------------------------------------------------------
    // Helper factories (inline, header-only)
    // -------------------------------------------------------------------------

    inline MessageHeader MakeHeader(MessageType type, uint32_t payloadSize = 0) noexcept {
        return MessageHeader{PROTOCOL_VERSION, type, payloadSize};
    }

    inline CommandResponsePayload MakeSuccess(MessageType orig, const char* msg = "") noexcept {
        CommandResponsePayload r{};
        r.originalCommand = orig;
        r.status          = CommandStatus::Success;
        if (msg) strncpy_s(r.message, msg, sizeof(r.message) - 1);
        return r;
    }

    inline CommandResponsePayload MakeFailure(MessageType orig, CommandStatus status, const char* msg = "") noexcept {
        CommandResponsePayload r{};
        r.originalCommand = orig;
        r.status          = status;
        if (msg) strncpy_s(r.message, msg, sizeof(r.message) - 1);
        return r;
    }

} // namespace VoiceClear::IPC
