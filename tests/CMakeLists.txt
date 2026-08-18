add_executable(VoiceClearBenchmarks 
    BenchmarkMain.cpp
    DeviceManagerTest.cpp
    CaptureEngineTest.cpp
    RingBufferTest.cpp
    OutputEngineTest.cpp
    AudioClockTest.cpp
    FormatConverterTest.cpp
    SampleRateConverterTest.cpp
    DeepFilterNetProcessorTest.cpp
    IpcClientTest.cpp
)

find_package(Catch2 CONFIG REQUIRED)

# Since we use Catch2 for benchmarking
target_link_libraries(VoiceClearBenchmarks PRIVATE Catch2::Catch2WithMain VoiceClearAudio VoiceClearDriver VoiceClearAI VoiceClearDiagnostics VoiceClearUtils VoiceClearBootstrap)
target_include_directories(VoiceClearBenchmarks PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)

add_executable(VoiceClearWavTool VoiceClearWavTool.cpp)
target_link_libraries(VoiceClearWavTool PRIVATE VoiceClearAI VoiceClearDSP VoiceClearUtils nlohmann_json::nlohmann_json)
target_include_directories(VoiceClearWavTool PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)
