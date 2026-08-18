find_package(kissfft CONFIG REQUIRED)

add_library(VoiceClearDSP STATIC
    STFT.cpp
    ErbFeatures.cpp
)

target_link_libraries(VoiceClearDSP PUBLIC
    CoreInterfaces
    VoiceClearCore
    VoiceClearUtils
    kissfft::kissfft-float
)

# Ensure kissfft include directory is available
find_path(KISSFFT_INCLUDE_DIR NAMES kissfft/kiss_fftr.h)
if(KISSFFT_INCLUDE_DIR)
    target_include_directories(VoiceClearDSP PUBLIC ${KISSFFT_INCLUDE_DIR})
endif()

target_include_directories(VoiceClearDSP PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
