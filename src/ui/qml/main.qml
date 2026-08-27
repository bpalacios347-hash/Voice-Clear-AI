import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import QtQuick.Controls.Material

Window {
    id: mainWindow
    width:  460
    height: 840
    minimumWidth:  460
    minimumHeight: 840
    visible: true
    title: "Voice Clear AI — Studio Noise Cancellation"
    color: "#0b0d14"

    onVisibleChanged: {
        viewModel.setWindowVisible(visible);
    }
    onVisibilityChanged: {
        viewModel.setWindowVisible(visibility !== Window.Minimized && visible);
    }

    Material.theme: Material.Dark
    Material.accent: Material.Teal

    // -------------------------------------------------------------------------
    // State & Connections
    // -------------------------------------------------------------------------
    property bool   diagnosticsVisible: false
    property string activeProfile: "Balanced"

    Connections {
        target: viewModel
        function onShowDiagnosticsPanel()  { diagnosticsVisible = !diagnosticsVisible }
        function onActiveProfileChanged(p) { activeProfile = p }
    }

    // =========================================================================
    // Background Surface
    // =========================================================================
    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#0d0f1a" }
            GradientStop { position: 0.5; color: "#090a12" }
            GradientStop { position: 1.0; color: "#06070d" }
        }
    }

    // Subtle ambient top glow
    Rectangle {
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        width: 320
        height: 120
        radius: 60
        color: viewModel.isMicrophoneEnabled ? "#00e5ff" : "#303a52"
        opacity: viewModel.isMicrophoneEnabled ? 0.08 : 0.03
        Behavior on color   { ColorAnimation { duration: 400 } }
        Behavior on opacity { NumberAnimation { duration: 400 } }
    }

    // =========================================================================
    // Main Scrollable / Column Layout
    // =========================================================================
    ScrollView {
        anchors.fill: parent
        anchors.margins: 22
        contentWidth: availableWidth
        clip: true
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

        ColumnLayout {
            width: parent.width
            spacing: 16

        // -----------------------------------------------------------------------
        // Header Bar
        // -----------------------------------------------------------------------
        RowLayout {
            Layout.fillWidth: true

            Column {
                spacing: 2
                Row {
                    spacing: 8
                    Text {
                        text: "Voice Clear AI"
                        color: "#ffffff"
                        font.pixelSize: 19
                        font.weight: Font.Bold
                        font.family: "Segoe UI"
                        font.letterSpacing: 0.5
                    }
                    Rectangle {
                        width: 28; height: 16; radius: 4
                        color: "#1e293b"
                        anchors.verticalCenter: parent.verticalCenter
                        Text {
                            anchors.centerIn: parent
                            text: "PRO"
                            color: "#38bdf8"
                            font.pixelSize: 9
                            font.weight: Font.Bold
                            font.family: "Segoe UI"
                        }
                    }
                }
                Text {
                    text: "Neural DeepFilterNet DSP Engine"
                    color: "#64748b"
                    font.pixelSize: 11
                    font.family: "Segoe UI"
                }
            }

            Item { Layout.fillWidth: true }

            // Service Connection Badge
            Rectangle {
                height: 28
                implicitWidth: badgeRow.implicitWidth + 18
                radius: 14
                color: viewModel.serviceConnected ? "#064e3b" : "#450a0a"
                border.color: viewModel.serviceConnected ? "#10b981" : "#ef4444"
                border.width: 1

                Row {
                    id: badgeRow
                    anchors.centerIn: parent
                    spacing: 6

                    Rectangle {
                        width: 8; height: 8; radius: 4
                        color: viewModel.serviceConnected ? "#34d399" : "#f87171"
                        anchors.verticalCenter: parent.verticalCenter

                        SequentialAnimation on opacity {
                            running: viewModel.serviceConnected
                            loops:   Animation.Infinite
                            NumberAnimation { to: 0.3; duration: 1000; easing.type: Easing.InOutQuad }
                            NumberAnimation { to: 1.0; duration: 1000; easing.type: Easing.InOutQuad }
                        }
                    }

                    Text {
                        text: viewModel.serviceConnected ? "WASAPI Active" : "Engine Offline"
                        color: viewModel.serviceConnected ? "#a7f3d0" : "#fca5a5"
                        font.pixelSize: 11
                        font.weight: Font.Medium
                        font.family: "Segoe UI"
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }
            }
        }

        // -----------------------------------------------------------------------
        // Central AI Power Button Hero
        // -----------------------------------------------------------------------
        Item {
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 4
            width: 170; height: 170

            // Outer Pulse Ring
            Rectangle {
                id: pulseRing
                anchors.centerIn: parent
                width:  parent.width
                height: parent.height
                radius: parent.width / 2
                color:  "transparent"
                border.color: viewModel.isMicrophoneEnabled ? "#00f2fe" : "#334155"
                border.width: 2
                opacity: viewModel.isMicrophoneEnabled ? 0.6 : 0.2

                SequentialAnimation on scale {
                    running: viewModel.isMicrophoneEnabled && viewModel.serviceConnected && mainWindow.visible && (mainWindow.visibility !== Window.Minimized)
                    loops:   Animation.Infinite
                    NumberAnimation { to: 1.10; duration: 1400; easing.type: Easing.InOutSine }
                    NumberAnimation { to: 1.00; duration: 1400; easing.type: Easing.InOutSine }
                }
            }

            // Main Interactive Circle
            Rectangle {
                id: micCircle
                anchors.centerIn: parent
                width:  150; height: 150
                radius: 75
                gradient: Gradient {
                    GradientStop {
                        position: 0.0
                        color: viewModel.isMicrophoneEnabled ? "#0ea5e9" : "#1e293b"
                    }
                    GradientStop {
                        position: 1.0
                        color: viewModel.isMicrophoneEnabled ? "#0284c7" : "#0f172a"
                    }
                }
                border.color: viewModel.isMicrophoneEnabled ? "#38bdf8" : "#334155"
                border.width: 2

                Behavior on border.color { ColorAnimation { duration: 300 } }

                scale: micMouseArea.pressed ? 0.94 : 1.0
                Behavior on scale { NumberAnimation { duration: 120 } }

                Column {
                    anchors.centerIn: parent
                    spacing: 6

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: viewModel.isMicrophoneEnabled ? "🛡️" : "⏸️"
                        font.pixelSize: 34
                    }

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: viewModel.isMicrophoneEnabled ? "NOISE CANCELED" : "BYPASSED"
                        color: "#ffffff"
                        font.pixelSize: 11
                        font.weight: Font.Bold
                        font.family: "Segoe UI"
                        font.letterSpacing: 1.0
                    }
                }

                MouseArea {
                    id: micMouseArea
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    enabled: viewModel.serviceConnected
                    onClicked: viewModel.setMicrophoneEnabled(!viewModel.isMicrophoneEnabled)
                }
            }
        }

        // Active Hardware Subtitle
        Column {
            Layout.alignment: Qt.AlignHCenter
            spacing: 2
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: viewModel.currentMicrophone.length > 0 ? viewModel.currentMicrophone : "Default Microphone"
                color: "#e2e8f0"
                font.pixelSize: 13
                font.weight: Font.DemiBold
                font.family: "Segoe UI"
                elide: Text.ElideMiddle
                maximumLineCount: 1
            }
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "DeepFilterNet 3 ONNX — 48 kHz Ultra-Low Latency"
                color: "#64748b"
                font.pixelSize: 10
                font.family: "Segoe UI"
            }
        }

        // -----------------------------------------------------------------------
        // Live Dual VU Meters & "Hear Myself" Monitor Card
        // -----------------------------------------------------------------------
        Rectangle {
            Layout.fillWidth: true
            height: 112
            color: "#131625"
            radius: 12
            border.color: "#1e243b"
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 8

                // Header with Hear Myself Toggle
                RowLayout {
                    Layout.fillWidth: true
                    Text {
                        text: "LIVE AUDIO METERS"
                        color: "#64748b"
                        font.pixelSize: 10
                        font.weight: Font.Bold
                        font.family: "Segoe UI"
                        font.letterSpacing: 1.2
                    }

                    Item { Layout.fillWidth: true }

                    // "Hear Myself" Monitor Button
                    Rectangle {
                        height: 24
                        implicitWidth: monitorRow.implicitWidth + 14
                        radius: 12
                        color: viewModel.isMonitorEnabled ? "#0284c7" : "#1e293b"
                        border.color: viewModel.isMonitorEnabled ? "#38bdf8" : "#334155"
                        border.width: 1

                        Row {
                            id: monitorRow
                            anchors.centerIn: parent
                            spacing: 5
                            Text {
                                text: "🎧"
                                font.pixelSize: 11
                                anchors.verticalCenter: parent.verticalCenter
                            }
                            Text {
                                text: viewModel.isMonitorEnabled ? "Hear Myself ON" : "Hear Myself"
                                color: viewModel.isMonitorEnabled ? "#ffffff" : "#94a3b8"
                                font.pixelSize: 10
                                font.weight: Font.DemiBold
                                font.family: "Segoe UI"
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: viewModel.toggleMonitor()
                        }
                    }
                }

                // Meter 1: Input Mic (Raw)
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8
                    Text {
                        text: "MIC IN:"
                        color: "#94a3b8"
                        font.pixelSize: 10
                        font.weight: Font.Medium
                        font.family: "Segoe UI"
                        Layout.preferredWidth: 54
                    }
                    Rectangle {
                        Layout.fillWidth: true
                        height: 10
                        radius: 5
                        color: "#0a0c14"
                        clip: true

                        Rectangle {
                            height: parent.height
                            radius: 5
                            width: Math.min(parent.width, parent.width * Math.max(0.02, viewModel.inputLevel))
                            gradient: Gradient {
                                orientation: Gradient.Horizontal
                                GradientStop { position: 0.0; color: "#10b981" }
                                GradientStop { position: 0.7; color: "#f59e0b" }
                                GradientStop { position: 1.0; color: "#ef4444" }
                            }
                            Behavior on width { NumberAnimation { duration: 60 } }
                        }
                    }
                }

                // Meter 2: Output Clean Voice (AI)
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8
                    Text {
                        text: "AI OUT:"
                        color: "#38bdf8"
                        font.pixelSize: 10
                        font.weight: Font.Bold
                        font.family: "Segoe UI"
                        Layout.preferredWidth: 54
                    }
                    Rectangle {
                        Layout.fillWidth: true
                        height: 10
                        radius: 5
                        color: "#0a0c14"
                        clip: true

                        Rectangle {
                            height: parent.height
                            radius: 5
                            width: Math.min(parent.width, parent.width * (viewModel.isMicrophoneEnabled ? Math.max(0.02, viewModel.outputLevel) : 0.0))
                            gradient: Gradient {
                                orientation: Gradient.Horizontal
                                GradientStop { position: 0.0; color: "#06b6d4" }
                                GradientStop { position: 0.8; color: "#3b82f6" }
                                GradientStop { position: 1.0; color: "#8b5cf6" }
                            }
                            Behavior on width { NumberAnimation { duration: 60 } }
                        }
                    }
                }
            }
        }

        // -----------------------------------------------------------------------
        // Audio Routing & Hardware Selector Card
        // -----------------------------------------------------------------------
        Rectangle {
            Layout.fillWidth: true
            height: 144
            color: "#131625"
            radius: 12
            border.color: "#1e243b"
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 8

                RowLayout {
                    Layout.fillWidth: true
                    Text {
                        text: "AUDIO ROUTING & BRIDGE"
                        color: "#64748b"
                        font.pixelSize: 10
                        font.weight: Font.Bold
                        font.family: "Segoe UI"
                        font.letterSpacing: 1.2
                    }
                    Item { Layout.fillWidth: true }
                    Text {
                        text: "🔄 Rescan"
                        color: "#38bdf8"
                        font.pixelSize: 10
                        font.weight: Font.DemiBold
                        font.family: "Segoe UI"
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: viewModel.refreshDevices()
                        }
                    }
                }

                // Input Microphone Dropdown
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Text {
                        text: "🎙️ Mic:"
                        color: "#94a3b8"
                        font.pixelSize: 11
                        font.family: "Segoe UI"
                        Layout.preferredWidth: 44
                    }

                    ComboBox {
                        id: inputCombo
                        Layout.fillWidth: true
                        Layout.preferredHeight: 30
                        model: viewModel.inputDevices.length > 0 ? viewModel.inputDevices : ["Default Microphone"]
                        currentIndex: viewModel.selectedInputIndex >= 0 ? viewModel.selectedInputIndex : 0
                        onActivated: (index) => viewModel.selectInputDevice(index)

                        background: Rectangle {
                            color: "#1a1f33"
                            radius: 6
                            border.color: inputCombo.hovered ? "#38bdf8" : "#2a344f"
                            border.width: 1
                        }

                        contentItem: Text {
                            leftPadding: 8
                            rightPadding: 20
                            text: inputCombo.displayText
                            color: "#ffffff"
                            font.pixelSize: 11
                            font.family: "Segoe UI"
                            verticalAlignment: Text.AlignVCenter
                            elide: Text.ElideRight
                        }
                    }
                }

                // Output Target Dropdown
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Text {
                        text: "🔊 Out:"
                        color: "#94a3b8"
                        font.pixelSize: 11
                        font.family: "Segoe UI"
                        Layout.preferredWidth: 44
                    }

                    ComboBox {
                        id: outputCombo
                        Layout.fillWidth: true
                        Layout.preferredHeight: 30
                        model: viewModel.outputDevices.length > 0 ? viewModel.outputDevices : ["CABLE Input (VB-Audio Virtual Cable)"]
                        currentIndex: viewModel.selectedOutputIndex >= 0 ? viewModel.selectedOutputIndex : 0
                        onActivated: (index) => viewModel.selectOutputDevice(index)

                        background: Rectangle {
                            color: "#1a1f33"
                            radius: 6
                            border.color: outputCombo.hovered ? "#10b981" : "#2a344f"
                            border.width: 1
                        }

                        contentItem: Text {
                            leftPadding: 8
                            rightPadding: 20
                            text: outputCombo.displayText
                            color: "#ffffff"
                            font.pixelSize: 11
                            font.family: "Segoe UI"
                            verticalAlignment: Text.AlignVCenter
                            elide: Text.ElideRight
                        }
                    }
                }

                // Guidance Hint
                Text {
                    Layout.fillWidth: true
                    text: "💡 In Discord, Zoom & OBS: Select 'CABLE Output' as your Input Device."
                    color: "#64748b"
                    font.pixelSize: 10
                    font.family: "Segoe UI"
                    font.italic: true
                    elide: Text.ElideRight
                }
            }
        }

        // -----------------------------------------------------------------------
        // Noise Profile Selector
        // -----------------------------------------------------------------------
        Rectangle {
            Layout.fillWidth: true
            height: 74
            color: "#131625"
            radius: 12
            border.color: "#1e243b"
            border.width: 1

            Column {
                anchors.fill:    parent
                anchors.margins: 12
                spacing: 8

                Text {
                    text: "AI NOISE SUPPRESSION PROFILE"
                    color: "#64748b"
                    font.pixelSize: 10
                    font.weight: Font.Bold
                    font.family: "Segoe UI"
                    font.letterSpacing: 1.2
                }

                Row {
                    spacing: 8
                    width: parent.width

                    Repeater {
                        model: ["Balanced", "Strong", "Voice", "Low CPU"]

                        Rectangle {
                            width:  (parent.width - 24) / 4
                            height: 30
                            radius: 6
                            color: activeProfile === modelData ? "#0369a1" : "#1a1f33"
                            border.color: activeProfile === modelData ? "#38bdf8" : "#2a344f"
                            border.width: 1

                            Behavior on color        { ColorAnimation { duration: 180 } }
                            Behavior on border.color { ColorAnimation { duration: 180 } }

                            Text {
                                anchors.centerIn: parent
                                text: modelData
                                color: activeProfile === modelData ? "#ffffff" : "#94a3b8"
                                font.pixelSize: 10
                                font.weight: activeProfile === modelData ? Font.Bold : Font.Normal
                                font.family: "Segoe UI"
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                enabled: viewModel.serviceConnected
                                onClicked: {
                                    var id = modelData === "Voice" ? "Voice Preservation" : modelData
                                    activeProfile = modelData
                                    viewModel.setProfile(id)
                                }
                            }
                        }
                    }
                }
            }
        }

        // -----------------------------------------------------------------------
        // Diagnostics Panel (Expandable Stats)
        // -----------------------------------------------------------------------
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: diagnosticsVisible ? 150 : 0
            color: "#131625"
            radius: 12
            border.color: "#1e243b"
            border.width: 1
            clip: true
            visible: Layout.preferredHeight > 0

            Behavior on Layout.preferredHeight { NumberAnimation { duration: 220; easing.type: Easing.OutCubic } }

            Grid {
                anchors.fill:    parent
                anchors.margins: 14
                columns:  2
                rows:     4
                rowSpacing:    8
                columnSpacing: 36

                Text { text: "Inference Latency"; color: "#64748b"; font.pixelSize: 11; font.family: "Segoe UI" }
                Text {
                    text: viewModel.p99Latency
                    font.pixelSize: 11; font.family: "Segoe UI"; font.weight: Font.DemiBold
                    color: {
                        var ms = parseFloat(viewModel.p99Latency)
                        return ms < 10 ? "#34d399" : ms < 15 ? "#fbbf24" : "#f87171"
                    }
                }

                Text { text: "Inference CPU"; color: "#64748b"; font.pixelSize: 11; font.family: "Segoe UI" }
                Text { text: viewModel.cpuUsage; color: "#e2e8f0"; font.pixelSize: 11; font.family: "Segoe UI"; font.weight: Font.DemiBold }

                Text { text: "RAM Working Set"; color: "#64748b"; font.pixelSize: 11; font.family: "Segoe UI" }
                Text { text: viewModel.memoryUsage; color: "#e2e8f0"; font.pixelSize: 11; font.family: "Segoe UI"; font.weight: Font.DemiBold }

                Text { text: "Audio XRUNs (Drops)"; color: "#64748b"; font.pixelSize: 11; font.family: "Segoe UI" }
                Text {
                    text: viewModel.xrunCount
                    font.pixelSize: 11; font.family: "Segoe UI"; font.weight: Font.DemiBold
                    color: parseInt(viewModel.xrunCount) === 0 ? "#34d399" : "#f87171"
                }
            }
        }

        // -----------------------------------------------------------------------
        // Bottom Footer
        // -----------------------------------------------------------------------
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            // Driver Mode Pill
            Rectangle {
                height: 24
                implicitWidth: driverText.implicitWidth + 16
                radius: 12
                color: "#1e293b"
                border.color: "#334155"
                border.width: 1

                Text {
                    id: driverText
                    anchors.centerIn: parent
                    text: !viewModel.serviceConnected ? "Offline" :
                          (viewModel.driverMode === 1 ? "AVStream Direct" : "WASAPI Stream")
                    color: !viewModel.serviceConnected ? "#f87171" : "#38bdf8"
                    font.pixelSize: 10
                    font.family: "Segoe UI"
                    font.weight: Font.DemiBold
                }
            }

            Item { Layout.fillWidth: true }

            // Support Button
            Rectangle {
                height: 24
                implicitWidth: supportText.implicitWidth + 18
                radius: 12
                color: supportPopup.visible ? "#0369a1" : "#1e293b"
                border.color: supportPopup.visible ? "#38bdf8" : "#334155"
                border.width: 1

                Text {
                    id: supportText
                    anchors.centerIn: parent
                    text: "📩 Soporte"
                    color: supportPopup.visible ? "#ffffff" : "#38bdf8"
                    font.pixelSize: 10
                    font.family: "Segoe UI"
                    font.weight: Font.DemiBold
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: supportPopup.open()
                }
            }

            // Stats Toggle Button
            Rectangle {
                height: 24
                implicitWidth: statsText.implicitWidth + 18
                radius: 12
                color: diagnosticsVisible ? "#0284c7" : "#1e293b"
                border.color: diagnosticsVisible ? "#38bdf8" : "#334155"
                border.width: 1

                Text {
                    id: statsText
                    anchors.centerIn: parent
                    text: diagnosticsVisible ? "▲ Hide Stats" : "▼ Show Stats"
                    color: diagnosticsVisible ? "#ffffff" : "#94a3b8"
                    font.pixelSize: 10
                    font.family: "Segoe UI"
                    font.weight: Font.DemiBold
                }
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: diagnosticsVisible = !diagnosticsVisible
                }
            }
        }
    }
}

    // =========================================================================
    // Support Contact Modal Popup
    // =========================================================================
    Popup {
        id: supportPopup
        anchors.centerIn: parent
        width: 380
        height: 260
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Rectangle {
            color: "#111422"
            radius: 14
            border.color: "#38bdf8"
            border.width: 1.5
        }

        property bool copied: false

        contentItem: ColumnLayout {
            anchors.fill: parent
            anchors.margins: 18
            spacing: 12

            RowLayout {
                spacing: 8
                Text {
                    text: "🛠️ Soporte Técnico"
                    color: "#ffffff"
                    font.pixelSize: 15
                    font.weight: Font.Bold
                    font.family: "Segoe UI"
                }
                Item { Layout.fillWidth: true }
                Text {
                    text: "✕"
                    color: "#94a3b8"
                    font.pixelSize: 14
                    font.weight: Font.Bold
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: supportPopup.close()
                    }
                }
            }

            Text {
                Layout.fillWidth: true
                text: "¿Tienes algún problema, duda o sugerencia con la cancelación de ruido o enrutamiento de audio?"
                color: "#cbd5e1"
                font.pixelSize: 11
                font.family: "Segoe UI"
                wrapMode: Text.WordWrap
            }

            // Email Card Container
            Rectangle {
                Layout.fillWidth: true
                height: 44
                radius: 8
                color: "#1a2035"
                border.color: "#334155"
                border.width: 1

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 8

                    Text {
                        text: "📧"
                        font.pixelSize: 14
                    }

                    Text {
                        id: emailField
                        Layout.fillWidth: true
                        text: "bpalacios347@gmail.com"
                        color: "#38bdf8"
                        font.pixelSize: 12
                        font.weight: Font.DemiBold
                        font.family: "Segoe UI"
                    }

                    Rectangle {
                        height: 26
                        width: 70
                        radius: 6
                        color: supportPopup.copied ? "#059669" : "#0284c7"

                        Text {
                            anchors.centerIn: parent
                            text: supportPopup.copied ? "✓ Copiado" : "Copiar"
                            color: "#ffffff"
                            font.pixelSize: 10
                            font.weight: Font.Bold
                            font.family: "Segoe UI"
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                copyHelper.text = "bpalacios347@gmail.com"
                                copyHelper.selectAll()
                                copyHelper.copy()
                                supportPopup.copied = true
                                copyResetTimer.restart()
                            }
                        }
                    }
                }
            }

            Text {
                Layout.fillWidth: true
                text: "Escribe a este correo y recibirás asistencia técnica personalizada."
                color: "#64748b"
                font.pixelSize: 10
                font.family: "Segoe UI"
                font.italic: true
                wrapMode: Text.WordWrap
            }

            Item { Layout.fillHeight: true }
        }
    }

    TextEdit {
        id: copyHelper
        visible: false
    }

    Timer {
        id: copyResetTimer
        interval: 2000
        onTriggered: supportPopup.copied = false
    }
}
