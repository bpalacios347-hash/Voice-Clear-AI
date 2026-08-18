cmake_minimum_required(VERSION 3.25)

project(VoiceClearAI
    VERSION 1.0.0
    DESCRIPTION "Professional Windows desktop application providing real-time microphone noise cancellation"
    LANGUAGES CXX C
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable IDE folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Configure output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Find Qt6 (Expects CMAKE_PREFIX_PATH or QT_DIR)
# find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Declarative Network)

# Find ONNX Runtime (Downloaded as pre-compiled binaries)
find_path(ONNXRUNTIME_INCLUDE_DIR NAMES onnxruntime_cxx_api.h PATHS "${CMAKE_SOURCE_DIR}/external/onnxruntime/include" REQUIRED)
find_library(ONNXRUNTIME_LIBRARY NAMES onnxruntime PATHS "${CMAKE_SOURCE_DIR}/external/onnxruntime/lib" REQUIRED)

# Fast vcpkg packages
find_package(spdlog CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

option(ENABLE_PROFILING "Enable Tracy Profiler Integration" OFF)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(ENABLE_PROFILING ON CACHE BOOL "Enable Tracy Profiler Integration" FORCE)
endif()

if(ENABLE_PROFILING)
    find_package(Tracy CONFIG REQUIRED)
    add_compile_definitions(ENABLE_PROFILING)
endif()

# Common compiler flags for MSVC
if(MSVC)
    add_compile_options(
        /W4
        /permissive-
        /Zc:__cplusplus
        /EHsc
    )
    
    # Enable Static Analysis in MSVC
    # add_compile_options(/analyze)
    
    # Enable AddressSanitizer (ASAN) in Debug builds
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(/fsanitize=address)
    endif()

    add_compile_definitions(
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        _UNICODE
        UNICODE
    )
endif()

# Core Interface Library (Headers Only)
add_library(CoreInterfaces INTERFACE)
target_include_directories(CoreInterfaces INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(CoreInterfaces INTERFACE nlohmann_json::nlohmann_json spdlog::spdlog fmt::fmt)

add_subdirectory(src)
add_subdirectory(tests)

# --- Post-build: Ensure all required runtime DLLs are copied alongside executables ---
if(TARGET VoiceClearService)
    add_custom_target(CopyRuntimeDlls ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
            "C:/Users/Byron/vcpkg/packages/spdlog_x64-windows/bin"
            "$<TARGET_FILE_DIR:VoiceClearService>"
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
            "C:/Users/Byron/vcpkg/packages/fmt_x64-windows/bin"
            "$<TARGET_FILE_DIR:VoiceClearService>"
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
            "C:/Users/Byron/vcpkg/packages/kissfft_x64-windows/bin"
            "$<TARGET_FILE_DIR:VoiceClearService>"
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
            "C:/Users/Byron/vcpkg/packages/re2_x64-windows/bin"
            "$<TARGET_FILE_DIR:VoiceClearService>"
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
            "${CMAKE_SOURCE_DIR}/external/onnxruntime/lib"
            "$<TARGET_FILE_DIR:VoiceClearService>"
        COMMENT "Copying runtime DLLs (spdlog, fmt, kissfft, onnxruntime) to output directory..."
    )
endif()

# --- Packaging & Release (WiX MSI) ---
# install(TARGETS VoiceClearService VoiceClear DESTINATION bin)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/models/deepfilternet3.onnx" DESTINATION bin/models)

set(CPACK_GENERATOR "WIX")
set(CPACK_PACKAGE_NAME "Voice Clear AI")
set(CPACK_PACKAGE_VENDOR "Byron")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_WIX_UPGRADE_GUID "D98A9B5D-9271-48CD-901D-21AE7CD4E5E1")
set(CPACK_WIX_ROOT_FEATURE_TITLE "Voice Clear AI Core")
set(CPACK_WIX_ROOT_FEATURE_DESCRIPTION "Installs the Voice Clear Windows Service, UI, and Virtual Audio Driver.")

include(CPack)
