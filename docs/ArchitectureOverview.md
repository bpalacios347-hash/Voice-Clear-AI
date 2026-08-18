# Architecture Overview

Voice Clear AI is composed of four primary system boundaries separated for security, reliability, and real-time performance.

## 1. The Windows Service (VoiceClearService.exe)
Runs as `LocalSystem`. It owns the DSP Pipeline, configures MMCSS Pro Audio threading limits, manages the AI execution, and hosts the IPC Server.
## 2. The Audio Pipeline (Real-Time Subsystem)
Lock-free, zero-allocation ring buffers that move audio chunks from the WASAPI hardware buffer, through the Format/Resampler, into the DeepFilterNet ONNX inference engine, and out to the virtual driver.
## 3. The Virtual Audio Driver (Kernel Mode)
A WDM Virtual Audio Device that exposes a virtual microphone to Windows OS applications (Zoom, Discord).
## 4. The UI (VoiceClear.exe)
A standard-privilege Qt6 QML application. Communicates with the background service exclusively via `\\.\pipe\VoiceClearIPC`.
