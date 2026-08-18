add_library(VoiceClearAudio STATIC
    AudioEngine.cpp
    DeviceManager.cpp
    CaptureEngine.cpp
    OutputEngine.cpp
    AudioClock.cpp
    FormatConverter.cpp
    SampleRateConverter.cpp
    AudioPipeline.cpp
)

target_link_libraries(VoiceClearAudio PUBLIC CoreInterfaces VoiceClearCore VoiceClearUtils VoiceClearDiagnostics VoiceClearProfiling)
if(WIN32)
    target_link_libraries(VoiceClearAudio PRIVATE Mmdevapi.lib Propsys.lib Ole32.lib Avrt.lib)
endif()
target_include_directories(VoiceClearAudio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
