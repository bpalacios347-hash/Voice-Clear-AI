# Voice Clear AI - Virtual Audio Driver

This directory contains the source code for the AVStream (KS) Virtual Audio Device.

## Architecture
- Based strictly on the Microsoft Virtual Audio Device (sysvad) WDK sample.
- Minimal transport layer.
- **Zero DSP** or AI inference happens in kernel space. All audio processing is handled by the user-mode Windows Service via shared memory / IOCTLs.
- Target: WHQL Certification readiness.

## Build Requirements
- Windows Driver Kit (WDK)
- Visual Studio with "Desktop development with C++" and "Windows Driver Kit" workloads.
- Test Signing must be enabled to install during development (`bcdedit /set testsigning on`).
