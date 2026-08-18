# Troubleshooting Guide

### 1. UI Shows "Connecting to Service..." forever
- Open `services.msc` and verify that **Voice Clear AI Core Engine** is running.
- If it is stopped, manually restart it.

### 2. Audio is glitching or popping
- This indicates the CPU is falling behind the real-time deadline.
- Ensure your PC supports AVX2 instructions (Intel Core Gen 4+ or AMD Ryzen).
- Open the Diagnostics drawer in the UI to check for XRUNs.

### 3. Crash Dumps
- If the service crashes, it will automatically save a `.dmp` file to `C:\ProgramData\VoiceClear\CrashDumps\`.
- Check `C:\ProgramData\VoiceClear\Logs\` for execution traces.
