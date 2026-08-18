import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: diagnosticsWindow
    visible: true
    width: 600
    height: 400
    title: "Voice Clear AI - Developer Diagnostics"
    
    // In a real application, this would be bound to C++ backend models via Q_PROPERTY
    property string activeModel: "DeepFilterNet3 ONNX (CPU)"
    property real currentLatency: 12.4
    property real inferenceTime: 8.2
    property real cpuUsage: 4.5
    property int xrunCount: 0

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 10

        Text {
            text: "Real-Time Telemetry"
            font.pixelSize: 20
            font.bold: true
        }

        GridLayout {
            columns: 2
            rowSpacing: 10
            columnSpacing: 20

            Text { text: "Active Model:" }
            Text { text: activeModel; font.bold: true }

            Text { text: "End-to-End Latency:" }
            Text { text: currentLatency.toFixed(2) + " ms"; color: currentLatency > 15 ? "red" : "green" }

            Text { text: "Inference Time:" }
            Text { text: inferenceTime.toFixed(2) + " ms" }

            Text { text: "CPU Usage:" }
            Text { text: cpuUsage.toFixed(1) + " %" }

            Text { text: "XRUNs / Drops:" }
            Text { text: xrunCount.toString(); color: xrunCount > 0 ? "red" : "black" }
        }

        Item { Layout.fillHeight: true } // spacer
    }
}
