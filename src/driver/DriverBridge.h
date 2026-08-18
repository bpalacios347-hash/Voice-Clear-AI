#pragma once
#include <string>
#include <memory>
#include <atomic>
#include <windows.h>
#include "audio/interfaces/IAudioStage.h"
#include "../diagnostics/TelemetryManager.h"

#include "common/VoiceClearSharedProtocol.h"

namespace VoiceClear::Driver {

    /**
     * @brief Abstract transport layer interface for the AVStream Virtual Microphone Driver.
     */
    class IDriverBridge {
    public:
        virtual ~IDriverBridge() = default;
        virtual bool Connect() = 0;
        virtual void Disconnect() = 0;
        virtual bool SendAudio(const Audio::AudioBuffer& buffer) noexcept = 0;
        virtual bool IsConnected() const noexcept = 0;
    };

    /**
     * @brief Real Driver Bridge that maps shared memory and connects to the WDM driver.
     */
    class RealDriverBridge : public IDriverBridge {
    public:
        explicit RealDriverBridge(std::shared_ptr<Diagnostics::TelemetryManager> telemetry);
        ~RealDriverBridge() override;

        bool Connect() override;
        void Disconnect() override;
        bool SendAudio(const Audio::AudioBuffer& buffer) noexcept override;
        bool IsConnected() const noexcept override { return m_connected.load(std::memory_order_relaxed); }

    private:
        std::shared_ptr<Diagnostics::TelemetryManager> m_telemetry;
        std::atomic<bool> m_connected{false};
        
        HANDLE m_hDevice{INVALID_HANDLE_VALUE};
        HANDLE m_hSharedMemory{nullptr};
        void* m_pSharedMemoryView{nullptr};

        Protocol::SharedMemoryHeader* m_pHeader{nullptr};
        float* m_pPayload{nullptr};

        bool InitializeSharedMemory();
    };

    /**
     * @brief Mock Driver Bridge for development when the kernel driver is not installed.
     * Prevents IOCTL failures from crashing the engine.
     */
    class MockDriverBridge : public IDriverBridge {
    public:
        explicit MockDriverBridge(std::shared_ptr<Diagnostics::TelemetryManager> telemetry);
        ~MockDriverBridge() override;

        bool Connect() override;
        void Disconnect() override;
        bool SendAudio(const Audio::AudioBuffer& buffer) noexcept override;
        bool IsConnected() const noexcept override { return m_connected.load(std::memory_order_relaxed); }

    private:
        std::shared_ptr<Diagnostics::TelemetryManager> m_telemetry;
        std::atomic<bool> m_connected{false};
    };

}
