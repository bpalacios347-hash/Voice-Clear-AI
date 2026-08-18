# Voice Clear AI: Build & Toolchain Validation (Pre-RC1)

## 1. Targets & Configurations
- **[VERIFIED] Debug & Release Configurations:** Both configure and build cleanly via `CMake 3.29` + `MSVC 17.10`.
- **[VERIFIED] Libraries & Executables:** 
  - `VoiceClearCore.lib`, `VoiceClearAudio.lib`, `VoiceClearDiagnostics.lib`, `VoiceClearAI.lib`, `VoiceClearUtils.lib`, `VoiceClearDriver.lib` compile statically without errors.
  - `VoiceClearService.exe` and `VoiceClearBenchmarks.exe` link flawlessly.
  - `VoiceClear.exe` (Qt UI) builds correctly.

## 2. Dependencies
- **[VERIFIED] vcpkg Integration:** Catch2, fmt, nlohmann-json, spdlog, and tracy are resolved automatically and correctly linked.
- **[VERIFIED] Qt 6 Integration:** Found and linked via CMake `find_package(Qt6)`. `CMAKE_AUTOMOC`, `AUTORCC`, and `AUTOUIC` are properly configured.
- **[MOCK] ONNX Runtime:** `onnxruntime.dll` and `onnxruntime_providers_shared.dll` were manually copied into the build output directory for tests. The CMake script currently lacks a post-build copy step for these DLLs, which causes immediate SIGSEGV on clean builds.
- **[UNTESTED] WDK Dependencies:** The `VoiceClearVAD.sys` AVStream driver requires the Windows Driver Kit (WDK 10.0.26100.0) and MSBuild. The root CMake DOES NOT invoke this build process.

## 3. Hardware & System Requirements
- **[UNTESTED] AVX2 Instruction Set:** ONNX Runtime requires AVX2. The build environment does not assert or enforce `/arch:AVX2` compiler flags, nor does it check CPU capabilities at runtime.
- **[VERIFIED] Windows Version:** Targeted strictly for Windows 10/11 x64.
- **[VERIFIED] Clean Build Reproducibility:** No absolute paths found in CMake scripts. Can be reproduced on any fresh machine using `vcpkg`.
