find_package(Qt6 COMPONENTS Core Gui Qml Quick QuickControls2 Widgets Network QUIET)

if(NOT Qt6_FOUND)
    message(WARNING "Qt6 not found. Skipping VoiceClear UI compilation.")
    return()
endif()

message(STATUS "Qt6 found at ${Qt6_DIR} — building VoiceClear UI")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

qt_add_executable(VoiceClear
    main.cpp
    IpcClient.cpp
    MainViewModel.cpp
    SystemTrayManager.cpp
    SettingsManager.cpp
    app_icon.rc
)

# Embed QML resources directly into the binary (faster startup, no loose QML files)
qt_add_qml_module(VoiceClear
    URI VoiceClearUI
    VERSION 1.0
    QML_FILES
        qml/main.qml
    RESOURCES
        qml.qrc
)

target_link_libraries(VoiceClear PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Widgets
    Qt6::Network
    VoiceClearBootstrap
    VoiceClearCore
    VoiceClearAudio
    VoiceClearDiagnostics
    VoiceClearAI
    VoiceClearDSP
    VoiceClearDriver
    VoiceClearPlatform
    ${ONNXRUNTIME_LIBRARY}
)

# Hide the console window in Release builds (Windows subsystem)
set_target_properties(VoiceClear PROPERTIES
    WIN32_EXECUTABLE TRUE
)

# Post-build: run windeployqt to copy all Qt DLLs next to the executable
find_program(WINDEPLOYQT windeployqt HINTS "${Qt6_DIR}/../../../bin")
if(WINDEPLOYQT)
    add_custom_command(TARGET VoiceClear POST_BUILD
        COMMAND ${WINDEPLOYQT}
            --qmldir "${CMAKE_CURRENT_SOURCE_DIR}/qml"
            --no-translations
            "$<TARGET_FILE:VoiceClear>"
        COMMENT "Running windeployqt for VoiceClear.exe"
    )
endif()
