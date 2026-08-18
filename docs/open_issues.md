# Voice Clear AI: Open Issues & Production Risks

This document contains a transparent evaluation of known limitations, technical debt, and pending features that pose risks to production stability.

## 1. Mock Components (Critical)
- **`MockDriverBridge` is currently active:** Audio is discarded into `/dev/null` in `EngineBootstrap`. Real kernel communication (AVStream properties, KS pins, IOCTLs) has NOT been end-to-end tested.
- **Model Loading:** Validates purely against `models.json` instead of a robust cryptographic hash verification system.
- **Auto Updater & Crash Reporter:** Only exist as scaffolding and network mocks.

## 2. Unimplemented Features (High)
- **Dynamic Profile Switching:** The wet/dry parameters are defined in theory (`NoiseProfile.h`), but `EngineBootstrap` and `ModelManager` lack a thread-safe update pathway to alter them in real-time.
- **Binary IPC Commands:** `IpcServer.cpp` reads commands assuming JSON via `IpcMessageType::CommandJson`. The binary protocol (`SetProfile`, `EnableProcessing`) needs to be fully mapped and parsed safely.
- **System Tray `VoiceClearUI`:** The frontend UI is detached from the system tray C++ scaffolding. 

## 3. Real-Time Architecture Risks (High)
- **AI Inference Isolation:** The ONNX inference currently executes directly on the capture thread. If the ONNX runtime spikes >15ms for a single frame, it will stall the capture ring buffer and cause audible XRUNs (dropouts). A dedicated inference worker thread with a decoupled lock-free exchange is highly recommended.

## 4. Hardware & Deployment Dependencies (Medium)
- **Kernel Mode Driver Signing:** The `VoiceClearVAD.sys` driver requires an EV Code Signing Certificate and WHQL certification to install on Windows 10/11 outside of Test Mode. 
- **AVX2 Requirement:** End-users running CPUs older than Intel 4th-Gen (Haswell) will experience immediate crashes. 

## 5. Summary
The engine is robust in its simulated vacuum, but the real-world interface layers (Kernel Driver and ONNX real-time scaling) present unresolved technical debt.
