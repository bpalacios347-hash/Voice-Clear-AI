# Voice Clear AI

Voice Clear AI is a professional, production-grade Windows desktop application providing real-time microphone noise cancellation. It runs entirely locally on the CPU (Intel 8th Gen+ optimized via AVX2/oneDNN) with an ultra-low latency target of 8-12 ms.

The application architecture separates the graphical interface (Qt 6) from a background Windows Service responsible for the real-time audio pipeline. It interfaces with an AVStream (KS) virtual audio device transport layer to expose the cleaned audio system-wide.

## Project Architecture

The system follows Clean Architecture principles, separating responsibilities into distinct modules:

- **src/service**: Background Windows Service running the real-time audio pipeline.
- **src/ui**: Qt 6-based minimal, modern graphical interface communicating with the service via IPC.
- **src/audio**: Audio core handling WASAPI communication and lock-free SPSC ring buffers.
- **src/ai**: Extensible AI processing engine leveraging ONNX Runtime and `DeepFilterNet3`.
- **src/driver**: The WDK AVStream Virtual Audio Device acting as a zero-DSP transport layer.
- **src/core**: Core abstractions (`IAudioProcessor`, `IModule`), IPC logic, and JSON configuration.
- **src/utils**: Logging (`spdlog`) and cross-cutting utilities.

## Build Instructions

### Prerequisites
- **Visual Studio 2022** with the following workloads:
  - Desktop development with C++
  - Windows Driver Kit (WDK) integration
- **CMake 3.25+**
- **vcpkg** (for dependency management)

### WDK Installation
To build the AVStream driver, you must install the Windows Driver Kit (WDK) for Windows 11. 
Ensure you install the WDK extension for Visual Studio.

### Building
The project relies on `vcpkg` in manifest mode to fetch Qt6, ONNX Runtime, spdlog, fmt, and nlohmann_json.

1. Configure the project using CMake presets:
   ```cmd
   cmake --preset windows-debug
   ```
2. Build the project:
   ```cmd
   cmake --build --preset debug
   ```

### Dependency Overview
- **Qt 6**: Used exclusively for the UI layer.
- **ONNX Runtime**: Executes the DeepFilterNet3 models utilizing the oneDNN provider for AVX2 CPU optimization.
- **spdlog & fmt**: Thread-safe, high-performance logging.
- **nlohmann_json**: Configuration file parsing (`settings.json`, `models.json`).
- **Catch2**: Unit testing framework (located in `/tests`).

## Future Roadmap

1. **Phase 1 (Current)**: Architecture Scaffolding, Clean Interfaces, vcpkg build setup, IPC foundation.
2. **Phase 2**: WASAPI capture pipeline, lock-free SPSC ring buffers, VAD integration.
3. **Phase 3**: ONNX Runtime AI Inference wrapper and audio limiter.
4. **Phase 4**: AVStream WDK driver development and deployment scripts.
5. **Phase 5**: Qt 6 UI development and final release packaging.
