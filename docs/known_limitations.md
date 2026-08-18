# Known Limitations (Pre-RC1)

**Date**: 2026-07-29

Based on a strict physical audit of the environment and repository, the following limitations prevent the project from being packaged as a fully complete end-to-end Release Candidate 1 (RC1).

## 1. AVStream Driver Not Buildable
- **Cause**: The Windows Driver Kit (WDK) toolchain (`WindowsKernelModeDriver10.0`) is not installed on the physical host machine.
- **Impact**: `src/driver/sys/VoiceClearVAD.vcxproj` cannot be compiled via MSBuild. The `.sys` binaries and the signed `.inf` files cannot be generated.
- **Mitigation**: The core service dynamically falls back to `MockDriverBridge` routing audio to `/dev/null`. To achieve a true RC1, a build agent with the correct WDK (or an EnterpriseWDK package) must be provisioned.

## 2. Qt6 UI Not Buildable
- **Cause**: The Qt6 framework is not installed or available in the system `PATH` (`C:\Qt` is missing).
- **Impact**: `add_subdirectory(ui)` fails during the CMake configure step. `VoiceClear.exe` cannot be compiled.
- **Mitigation**: The `ui` subdirectory was excluded from the main build script to allow the backend service to compile. A local Qt6 installation is strictly required to compile the interface.

## 3. Installer Not Compilable
- **Cause**: Inno Setup Compiler (`iscc.exe`) is not installed on the system.
- **Impact**: The generated `installer/setup.iss` script cannot be converted into an executable `.exe` installer automatically.
- **Mitigation**: The script is provided, but must be compiled manually on an environment featuring Inno Setup.

## Conclusion
The C++ Backend Service (`VoiceClearService.exe`) and AI logic are production-ready. However, the lack of WDK and Qt6 on the build machine explicitly prevents the generation of a complete RC1 installer that includes the UI and Driver.
