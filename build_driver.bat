@echo off
setlocal

:: Paths to Visual Studio and WDK
set "VC_VARS=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
set "WDK_INC=C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0"
set "WDK_LIB=C:\Program Files (x86)\Windows Kits\10\Lib\10.0.26100.0\km\x64"
set "SRC_DIR=src\driver\sys"
set "OUT_DIR=src\driver\sys\x64\Release"

if not exist "%OUT_DIR%" mkdir "%OUT_DIR%"

:: Initialize MSVC environment
call "%VC_VARS%" >nul

echo Compiling DriverEntry.cpp...
cl.exe /c /FI"warning.h" /FI"wdm.h" /I"%WDK_INC%\km" /I"%WDK_INC%\km\crt" /I"%WDK_INC%\shared" /I"%WDK_INC%\um" /D_AMD64_ /D_WIN64 /D_KERNEL_MODE /O2 /W3 /Zc:wchar_t /Zi /EHsc /Gy /Fo"%OUT_DIR%\DriverEntry.obj" %SRC_DIR%\DriverEntry.cpp
if %errorlevel% neq 0 exit /b %errorlevel%

echo Compiling Filter.cpp...
cl.exe /c /FI"warning.h" /FI"wdm.h" /I"%WDK_INC%\km" /I"%WDK_INC%\km\crt" /I"%WDK_INC%\shared" /I"%WDK_INC%\um" /D_AMD64_ /D_WIN64 /D_KERNEL_MODE /O2 /W3 /Zc:wchar_t /Zi /EHsc /Gy /Fo"%OUT_DIR%\Filter.obj" %SRC_DIR%\Filter.cpp
if %errorlevel% neq 0 exit /b %errorlevel%

echo Compiling Pin.cpp...
cl.exe /c /FI"warning.h" /FI"wdm.h" /I"%WDK_INC%\km" /I"%WDK_INC%\km\crt" /I"%WDK_INC%\shared" /I"%WDK_INC%\um" /D_AMD64_ /D_WIN64 /D_KERNEL_MODE /O2 /W3 /Zc:wchar_t /Zi /EHsc /Gy /Fo"%OUT_DIR%\Pin.obj" %SRC_DIR%\Pin.cpp
if %errorlevel% neq 0 exit /b %errorlevel%

echo Linking VoiceClearVAD.sys...
link.exe /DRIVER /SUBSYSTEM:NATIVE /ENTRY:DriverEntry /NODEFAULTLIB /OUT:"%OUT_DIR%\VoiceClearVAD.sys" /LIBPATH:"%WDK_LIB%" ntoskrnl.lib hal.lib ks.lib ksguid.lib wdm.lib "%OUT_DIR%\DriverEntry.obj" "%OUT_DIR%\Filter.obj" "%OUT_DIR%\Pin.obj"
if %errorlevel% neq 0 exit /b %errorlevel%

echo Compilation successful! Output at: %OUT_DIR%\VoiceClearVAD.sys
exit /b 0
