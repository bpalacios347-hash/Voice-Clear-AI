<#
.SYNOPSIS
Fast Setup for ONNX Runtime Pre-built Binaries
#>

$ErrorActionPreference = "Stop"
$OnnxUrl = "https://github.com/microsoft/onnxruntime/releases/download/v1.19.0/onnxruntime-win-x64-1.19.0.zip"
$ZipPath = "onnxruntime.zip"
$ExtractPath = "external\onnxruntime"

if (Test-Path $ExtractPath) {
    Write-Host "ONNX Runtime is already downloaded." -ForegroundColor Green
    exit 0
}

Write-Host "Downloading ONNX Runtime 1.19.0..." -ForegroundColor Cyan
Invoke-WebRequest -Uri $OnnxUrl -OutFile $ZipPath

Write-Host "Extracting..." -ForegroundColor Cyan
Expand-Archive -Path $ZipPath -DestinationPath "external" -Force
Rename-Item -Path "external\onnxruntime-win-x64-1.19.0" -NewName "onnxruntime"

Remove-Item $ZipPath
Write-Host "ONNX Runtime setup complete!" -ForegroundColor Green
