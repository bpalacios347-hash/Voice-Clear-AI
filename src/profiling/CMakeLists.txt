add_library(VoiceClearProfiling STATIC
    AIProfiler.cpp
)
if(ENABLE_PROFILING)
    target_link_libraries(VoiceClearProfiling PUBLIC Tracy::TracyClient)
endif()
target_include_directories(VoiceClearProfiling PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
