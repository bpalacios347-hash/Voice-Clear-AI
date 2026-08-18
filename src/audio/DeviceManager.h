#pragma once
#include <string>
#include <vector>
#include <functional>
#include <thread>
#include <atomic>
#include <memory>
#include "../diagnostics/LockFreeTelemetryQueue.h" // We can reuse the lock-free queue logic, or build a specific one

namespace VoiceClear::Audio {

    enum class DeviceState {
        Active,
        Disabled,
        NotPresent,
        Unplugged
    };

    enum class DataFlow {
        Render,
        Capture,
        All
    };

    struct AudioEndpointInfo {
        std::string id;
        std::string friendlyName;
        DeviceState state;
        DataFlow flow;
        bool isDefaultConsole;
        bool isDefaultCommunication;
    };

    enum class DeviceEventType {
        Added,
        Removed,
        StateChanged,
        DefaultChanged
    };

    struct DeviceEvent {
        DeviceEventType type;
        std::string deviceId;
        DataFlow flow;
    };

    /**
     * @brief Manages enumeration, hot-plugging, and default device changes for audio hardware.
     * Fully encapsulates Windows MMDevice API and COM threading.
     */
    class DeviceManager {
    public:
        using DeviceChangeCallback = std::function<void(const DeviceEvent&)>;

        DeviceManager();
        ~DeviceManager();

        // Lifecycle
        bool Initialize();
        bool Start();
        void Stop();
        void Shutdown();

        // Capabilities
        std::vector<AudioEndpointInfo> EnumerateInputDevices();
        std::vector<AudioEndpointInfo> EnumerateOutputDevices();
        
        // Hooks to trigger Audio Engine State Machine recovery (<2s target)
        void RegisterEventCallback(DeviceChangeCallback callback);

    private:
        void ComThreadLoop();
        void WorkerThreadLoop();

        class NotificationClient; // Internal IMMNotificationClient implementation
        std::unique_ptr<NotificationClient> m_notificationClient;

        std::thread m_comThread;
        std::thread m_workerThread;
        std::atomic<bool> m_running{false};
        std::atomic<bool> m_initialized{false};

        // Lock-free queue for routing COM events to the worker thread
        Diagnostics::LockFreeTelemetryQueue<DeviceEvent> m_eventQueue;
        
        DeviceChangeCallback m_callback;
    };

}
