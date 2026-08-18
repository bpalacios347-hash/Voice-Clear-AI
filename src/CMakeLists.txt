add_subdirectory(core)
add_subdirectory(utils)
add_subdirectory(profiling)
add_subdirectory(diagnostics)
add_subdirectory(audio)
add_subdirectory(driver)
add_subdirectory(ai)
add_subdirectory(dsp)
add_subdirectory(platform)

# ---- Engine bootstrap (composition root) ----
add_library(VoiceClearBootstrap STATIC EngineBootstrap.cpp)
target_link_libraries(VoiceClearBootstrap PUBLIC
    VoiceClearAudio
    VoiceClearAI
    VoiceClearDriver
    VoiceClearDiagnostics
    VoiceClearDSP
    VoiceClearCore
)
target_include_directories(VoiceClearBootstrap PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# ---- Service executable ----
add_library(VoiceClearServiceLib STATIC
    service/ServiceMain.cpp
    service/Watchdog.cpp
)
target_link_libraries(VoiceClearServiceLib PUBLIC
    VoiceClearBootstrap
    VoiceClearUtils
)

add_executable(VoiceClearService main.cpp)
target_link_libraries(VoiceClearService PRIVATE VoiceClearServiceLib)

# ---- Qt6 UI executable ----
# Qt6 installed at C:\Qt\6.7.3\msvc2019_64 via aqt
if(NOT DEFINED Qt6_DIR)
    set(Qt6_DIR "C:/Qt/6.7.3/msvc2019_64/lib/cmake/Qt6")
endif()

add_subdirectory(ui)
