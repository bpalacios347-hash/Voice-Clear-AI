add_library(VoiceClearCore STATIC
    config/ConfigManager.cpp
)

# IpcProtocol.h is a header-only protocol definition shared by the service and UI.
# We expose the ipc/ subdirectory as a public include so both VoiceClearDiagnostics
# (IpcServer) and VoiceClearUI (IpcClient) can include it as:
#   #include "core/ipc/IpcProtocol.h"

target_link_libraries(VoiceClearCore PUBLIC CoreInterfaces)
target_include_directories(VoiceClearCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/ipc
)
