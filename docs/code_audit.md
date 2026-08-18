# Voice Clear AI: Code Audit (Pre-RC1)

## 1. Mocks & Temporary Implementations
- **[MOCK] `MockDriverBridge` (src/driver/DriverBridge.cpp):** Audio is currently dumped to `/dev/null`. This needs to be replaced with the real `AvStreamDriverBridge` before RC1.
- **[MOCK] `ModelManager.cpp` (JSON Parsing):** Uses a dummy parse simulation instead of `nlohmann::json` or `rapidjson`.
- **[MOCK] `IpcServer.cpp`:** Telemetry variables `payload.memoryUsageMb` and `uptimeSeconds` are hardcoded to `58.0f` and `0.0f`.
- **[MOCK] `AutoUpdater.cpp`:** Simulates fetching JSON from `voiceclear.ai` and mocks SHA256 verification.
- **[MOCK] `CrashReporter.cpp`:** Mocks HTTP upload of minidumps.
- **[MOCK] `SystemTrayManager.h` (src/ui/):** Backend-only mock scaffolding identified.

## 2. Resource Leaks & Real-Time Considerations
- **[PARTIAL] Memory Allocations:** The audio path properly uses `LockFreeRingBuffer`. However, dynamic allocations still exist during component initialization and IPC JSON handling. IPC JSON command handling currently operates on a background thread which mitigates risk, but must be explicitly separated from the DSP thread.
- **[VERIFIED] Audio Thread Blocking:** No blocking calls exist in the `SampleRateConverter`, `InputGainStage`, or DSP pipeline. `LockFreeRingBuffer` achieves ~20ns wait-free swaps.
- **[UNTESTED] `DeepFilterNetProcessor` AI Threading:** Inference happens on the main DSP thread. While processing takes <15ms under benchmark, a dedicated AI inference worker thread should be considered to prevent buffer underruns if OS scheduling delays the thread.

## 3. Structural Concerns & Duplication
- **[PARTIAL] IPC JSON Parsing:** The current UI (`VoiceClear.exe`) uses IPC to communicate. The `IpcServer` handles JSON, but `IpcClientTest.cpp` handles binary handshakes. The protocol must be unified strictly to binary (`IpcMessageType`) per requirements.
- **[MISSING] Real Profile Swapping:** `EngineBootstrap.cpp` lacks a thread-safe atomic pointer exchange method to swap parameters in `ModelManager` while the engine is running.
- **[PARTIAL] Separation of Concerns:** `ServiceMain.cpp` directly handles console blocking logic and service status handling.

## 4. Orphan Files & Unused Code
- **[PARTIAL] `src/ui/SystemTrayManager.cpp`:** Exists in scaffolding but `QSystemTrayIcon` logic is not fully wired to `main.qml` or `main.cpp`.
- **[UNTESTED] `src/driver/sys/VoiceClearVAD.vcxproj`:** The WDK AVStream driver project exists but is decoupled from the main CMake build. It must be built via MSBuild independently.
