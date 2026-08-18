<#
.SYNOPSIS
Automated Build and Test Script for Voice Clear AI (Version 1.0.0)

.DESCRIPTION
This script automates the full compilation of the Voice Clear AI ecosystem:
1. Validates prerequisites (CMake, MSBuild, vcpkg)
2. Compiles the C++20 Core Service and Qt6 UI using CMake.
3. Compiles the AVStream Kernel Driver using MSBuild.
4. Executes the Catch2 unit tests.

.EXAMPLE
.\build_and_test.ps1
#>

$ErrorActionPreference = "Stop"

function Write-Step ($Message) {
    Write-Host "`n[====== $Message ======]`n" -ForegroundColor Cyan
}

function Check-Command ($CommandName, $InstallHint) {
    if (!(Get-Command $CommandName -ErrorAction SilentlyContinue)) {
        Write-Host "ERROR: $CommandName is missing." -ForegroundColor Red
        Write-Host "Please install $InstallHint and ensure it is in your PATH." -ForegroundColor Yellow
        exit 1
    }
}

Write-Step "Validating Prerequisites"
Check-Command "cmake" "CMake 3.25+"
Check-Command "msbuild" "Visual Studio 2022 (with MSVC and WDK)"

if (-not $env:VCPKG_ROOT) {
    Write-Host "ERROR: VCPKG_ROOT environment variable is not set." -ForegroundColor Red
    exit 1
}

Write-Step "Downloading Pre-built Dependencies"
.\setup_onnx.ps1

if (-not $env:QT_DIR) {
    Write-Host "WARNING: QT_DIR is not set. Assuming Qt is installed in PATH or standard location." -ForegroundColor Yellow
}

# ---------------------------------------------------------
# 1. Build User-Mode Application (Engine + UI)
# ---------------------------------------------------------
Write-Step "Building User-Mode Application (CMake)"
if (-not (Test-Path "build")) {
    New-Item -ItemType Directory -Path "build" | Out-Null
}

Push-Location "build"

Write-Host "Configuring CMake..." -ForegroundColor Green
cmake .. --preset default -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT\scripts\buildsystems\vcpkg.cmake"

Write-Host "Compiling CMake Targets (Release)..." -ForegroundColor Green
cmake --build . --config Release

Pop-Location

# ---------------------------------------------------------
# 2. Execute Catch2 Unit Tests
# ---------------------------------------------------------
Write-Step "Executing Unit Tests (CTest)"
Push-Location "build"
ctest -C Release --output-on-failure
Pop-Location

# ---------------------------------------------------------
# 3. Build Kernel-Mode AVStream Driver
# ---------------------------------------------------------
Write-Step "Building Kernel-Mode Driver (MSBuild)"
Push-Location "src\driver\sys"
msbuild VoiceClearVAD.vcxproj /p:Configuration=Release /p:Platform=x64 /t:Rebuild
Pop-Location

Write-Step "Build and Testing Complete!"
Write-Host "Binaries are located in: build/bin/Release/" -ForegroundColor Green
Write-Host "Driver is located in: src/driver/sys/x64/Release/" -ForegroundColor Green
Write-Host "`nTo install the virtual microphone driver (Requires Test Mode):"
Write-Host "pnputil /add-driver src\driver\sys\x64\Release\VoiceClearVAD.inf /install`n" -ForegroundColor Yellow
