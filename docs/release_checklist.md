# Voice Clear AI: Release Checklist (RC1)

| Task | Priority | Prerequisite For | Status |
| :--- | :--- | :--- | :--- |
| **1. Binary IPC Protocol (IpcProtocol.h)** | CRITICAL | RC1 | ✅ DONE |
| Canonical `src/core/ipc/IpcProtocol.h` created. Both `IpcServer` and `IpcClient` now use shared structs. Protocol bumped to v2 with `CommandResponse`, `StatusResponse`, `ReloadModel`, `GetStatus`. | | | |
| **2. Full Binary IPC Commands** | HIGH | RC1 | ✅ DONE |
| `IpcServer` dispatches `SetProfile`, `Enable/DisableProcessing`, `ReloadModel`, `GetStatus`. Every command returns a `CommandResponse`. Real DACL (Authenticated Users only). | | | |
| **3. IpcClient Refactor** | HIGH | RC1 | ✅ DONE |
| `IpcClient` uses `IpcProtocol.h`, handles `CommandResponse`, `StatusResponse`, `ShutdownUI`. Added `sendReloadModel()`, `sendGetStatus()`. Write mutex for thread-safety. | | | |
| **4. Inference Worker Thread** | MEDIUM | RC2 | ✅ DONE |
| `InferenceWorker` decouples ONNX from the capture thread. Runs on its own MMCSS "Pro Audio" scheduled thread. Lock-free SPSC ring exchange with 480-sample frames. Peak latency tracking. | | | |
| **5. SHA-256 Model Integrity Verification** | HIGH | RC1 | ✅ DONE |
| `ModelManager` now computes SHA-256 of the ONNX file via Windows BCrypt API and compares to `models.json`. Real hash of `deepfilternet3.onnx` written to manifest. | | | |
| **6. EngineBootstrap + ReloadModel** | HIGH | RC1 | ✅ DONE |
| Wired `InferenceWorker`. Added `ReloadModel()` for hot-swap at runtime. Exposes `GetActiveProfileName()` / `GetActiveModelName()` for telemetry. | | | |
| **7. Qt System Tray UI — Profile Selector** | HIGH | RC1 | ✅ DONE |
| `main.qml` redesigned: animated glow ring mic button, profile selector bar, expandable diagnostics with colour-coded latency, driver mode badge. `MainViewModel` exposes `setProfile`, `reloadModel`, `requestStatus` Q_INVOKABLEs. | | | |
| **8. Build Inno Setup Installer (`setup.iss`)** | HIGH | RC1 | ✅ DONE |
| Full script: OS version check, MSVC runtime check, AVX2 advisory, pnputil driver install, Qt6 DLLs section (commented), registry `InstallPath`, auto-start option, clean uninstall. | | | |
| **9. Real AvStreamDriverBridge** | CRITICAL | RC1 | ✅ DONE (previously) |
| `RealDriverBridge` uses shared memory protocol. `EngineBootstrap` attempts real bridge first, logs warning and continues if unavailable. | | | |
| **10. Physical 24-hour Endurance Test** | CRITICAL | RC1 | 🟡 PENDING |
| Requires signed driver + physical hardware. | | | |
| **11. EV Code Signing + WHQL** | CRITICAL | RC1 | 🔴 PENDING |
| EV certificate + Microsoft Hardware Dev Center submission required. | | | |
| **12. Qt6 + WDK Build Environment** | CRITICAL | RC1 | 🔴 PENDING |
| Qt6 and WDK must be installed on the build machine. | | | |
| **13. Real CrashReporter / AutoUpdater HTTP** | LOW | RC2 | 🔴 PENDING |
| Replace stubs with real HTTP upload/download implementations. | | | |
