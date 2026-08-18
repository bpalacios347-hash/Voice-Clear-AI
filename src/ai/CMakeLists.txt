add_library(VoiceClearAI STATIC
    InferenceContext.cpp
    ModelManager.cpp
    DeepFilterNetProcessor.cpp
    InferenceEngine.cpp
    InferenceWorker.cpp
)

target_link_libraries(VoiceClearAI PUBLIC
    CoreInterfaces
    VoiceClearCore
    VoiceClearUtils
    VoiceClearDiagnostics
    VoiceClearProfiling
    VoiceClearAudio
    ${ONNXRUNTIME_LIBRARY}
    avrt      # MMCSS (AvSetMmThreadCharacteristics)
    bcrypt    # SHA-256 via BCrypt API
)

target_include_directories(VoiceClearAI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${ONNXRUNTIME_INCLUDE_DIR})
