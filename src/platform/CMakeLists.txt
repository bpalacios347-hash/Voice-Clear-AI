add_library(VoiceClearPlatform STATIC
    # Windows specific code goes here
    PlatformDummy.cpp
)
target_link_libraries(VoiceClearPlatform PUBLIC CoreInterfaces)
target_include_directories(VoiceClearPlatform PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
