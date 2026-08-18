call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
"C:\Program Files\CMake\bin\cmake.exe" -B build -S . -DCMAKE_TOOLCHAIN_FILE="C:\Users\Byron\vcpkg\scripts\buildsystems\vcpkg.cmake"
"C:\Program Files\CMake\bin\cmake.exe" --build build --config Release
