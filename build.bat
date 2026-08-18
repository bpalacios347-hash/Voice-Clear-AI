@echo off
REM =============================================================================
REM build.bat — Configures and builds Voice Clear AI in Release x64
REM Run from the project root: build.bat
REM =============================================================================
setlocal enabledelayedexpansion

REM ---- Locate VS 2022 Community ----
set VSVARS="C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat"
if not exist %VSVARS% (
    echo ERROR: Visual Studio 2022 Community not found at %VSVARS%
    exit /b 1
)
call %VSVARS% amd64
echo [OK] MSVC x64 environment initialized.

REM ---- Qt6 location ----
set Qt6_DIR=C:\Qt\6.7.3\msvc2019_64\lib\cmake\Qt6
if not exist "%Qt6_DIR%" (
    echo WARNING: Qt6 not found at %Qt6_DIR% -- UI will not be built.
)

REM ---- vcpkg (VS-bundled, version aware) ----
set VCPKG_EXE=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\vcpkg\vcpkg.exe
set VCPKG_ROOT=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\vcpkg
set VCPKG_TOOLCHAIN=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake

echo [*] Installing vcpkg dependencies...
"%VCPKG_EXE%" install --triplet=x64-windows --vcpkg-root="%VCPKG_ROOT%"
if %ERRORLEVEL% NEQ 0 (
    echo WARNING: vcpkg install returned non-zero. Proceeding anyway (packages may already be cached).
)

REM ---- Remove old CMake cache ----
if exist build\CMakeCache.txt del /q build\CMakeCache.txt
if exist build\CMakeFiles rmdir /s /q build\CMakeFiles

REM ---- CMake Configure ----
echo [*] Running CMake configure...
cmake -B build -S . ^
    -G "Visual Studio 17 2022" ^
    -A x64 ^
    -DCMAKE_PREFIX_PATH="C:/Qt/6.7.3/msvc2019_64" ^
    -DQt6_DIR="C:/Qt/6.7.3/msvc2019_64/lib/cmake/Qt6" ^
    "-DCMAKE_TOOLCHAIN_FILE=%VCPKG_TOOLCHAIN%" ^
    -DVCPKG_TARGET_TRIPLET=x64-windows

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: CMake configure failed.
    exit /b 1
)

REM ---- CMake Build (Release) ----
echo [*] Building Release...
cmake --build build --config Release --parallel

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Build failed.
    exit /b 1
)

echo.
echo ================================================
echo  Build complete!
echo  Service:  build\bin\Release\VoiceClearService.exe
echo  UI:       build\bin\Release\VoiceClear.exe (if Qt6 found)
echo ================================================
