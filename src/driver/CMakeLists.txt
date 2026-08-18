add_library(VoiceClearDriver STATIC
    DriverBridge.cpp
)

target_link_libraries(VoiceClearDriver PUBLIC CoreInterfaces VoiceClearCore VoiceClearUtils VoiceClearDiagnostics VoiceClearProfiling VoiceClearAudio)

target_include_directories(VoiceClearDriver PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
