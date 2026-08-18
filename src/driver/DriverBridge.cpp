#include "DriverBridge.h"
#include "../utils/Logger.h"
#include "../profiling/ProfilingMacros.h"
#include <algorithm>

namespace VoiceClear::Driver {

    // --- RealDriverBridge ---

    RealDriverBridge::RealDriverBridge(std::shared_ptr<Diagnostics::TelemetryManager> telemetry)
        : m_telemetry(std::move(telemetry)) {}

    RealDriverBridge::~RealDriverBridge() { Disconnect(); }

    bool RealDriverBridge::Connect() {
        if (m_connected) return true;
        if (!InitializeSharedMemory()) {
            Utils::Logger::GetInstance()->Log(Core::LogLevel::Error, "Failed to initialize Shared Memory for Driver Bridge.");
            Disconnect();
            return false;
        }

        m_connected.store(true, std::memory_order_release);
        Utils::Logger::GetInstance()->Log(Core::LogLevel::Info, "RealDriverBridge connected successfully.");
        return true;
    }

    void RealDriverBridge::Disconnect() {
        m_connected.store(false, std::memory_order_release);
        if (m_pSharedMemoryView) { UnmapViewOfFile(m_pSharedMemoryView); m_pSharedMemoryView = nullptr; }
        if (m_hSharedMemory) { CloseHandle(m_hSharedMemory); m_hSharedMemory = nullptr; }
        if (m_hDevice != INVALID_HANDLE_VALUE) { CloseHandle(m_hDevice); m_hDevice = INVALID_HANDLE_VALUE; }
    }

    bool RealDriverBridge::InitializeSharedMemory() {
        const size_t memSize = Protocol::CalculateSharedMemorySize();
        m_hSharedMemory = CreateFileMappingA(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, static_cast<DWORD>(memSize), Protocol::SHARED_MEM_NAME);
        if (!m_hSharedMemory) return false;
        m_pSharedMemoryView = MapViewOfFile(m_hSharedMemory, FILE_MAP_ALL_ACCESS, 0, 0, memSize);
        if (!m_pSharedMemoryView) return false;

        m_pHeader = reinterpret_cast<Protocol::SharedMemoryHeader*>(m_pSharedMemoryView);
        m_pPayload = reinterpret_cast<float*>(reinterpret_cast<uint8_t*>(m_pSharedMemoryView) + sizeof(Protocol::SharedMemoryHeader));

        m_pHeader->protocolVersion = Protocol::SHARED_PROTOCOL_VERSION;
        m_pHeader->headerSize = sizeof(Protocol::SharedMemoryHeader);
        m_pHeader->sampleRate = Protocol::VAD_SAMPLE_RATE;
        m_pHeader->channels = Protocol::VAD_CHANNELS;
        m_pHeader->capacity = Protocol::VAD_BUFFER_CAPACITY_FRAMES;
        m_pHeader->timestamp = 0;
        m_pHeader->sequenceNumber = 0;

        m_pHeader->head.store(0, std::memory_order_relaxed);
        m_pHeader->tail.store(0, std::memory_order_relaxed);
        return true;
    }

    bool RealDriverBridge::SendAudio(const Audio::AudioBuffer& buffer) noexcept {
        VC_PROFILE_ZONE("RealDriverBridge_SendAudio");
        if (!m_connected.load(std::memory_order_acquire) || !m_pHeader) {
            // Driver is optional when using WASAPI / VB-Audio Virtual Cable mode
            return false;
        }
        
        const uint32_t head = m_pHeader->head.load(std::memory_order_relaxed);
        const uint32_t tail = m_pHeader->tail.load(std::memory_order_acquire);
        uint32_t capacity = m_pHeader->capacity;
        uint32_t available = (tail > head) ? (tail - head - 1) : (capacity - head + tail - 1);
        uint32_t toWrite = std::min(buffer.frames, available);

        if (toWrite == 0) {
            return false;
        }
        
        uint32_t firstChunk = std::min(toWrite, capacity - head);
        std::copy_n(buffer.samples.data(), firstChunk, m_pPayload + head);
        if (firstChunk < toWrite) std::copy_n(buffer.samples.data() + firstChunk, toWrite - firstChunk, m_pPayload);
        m_pHeader->head.store((head + toWrite) % capacity, std::memory_order_release);
        return true;
    }

    // --- MockDriverBridge ---
    
    MockDriverBridge::MockDriverBridge(std::shared_ptr<Diagnostics::TelemetryManager> telemetry)
        : m_telemetry(std::move(telemetry)) {}
        
    MockDriverBridge::~MockDriverBridge() { Disconnect(); }
    
    bool MockDriverBridge::Connect() {
        m_connected.store(true, std::memory_order_release);
        Utils::Logger::GetInstance()->Log(Core::LogLevel::Info, "MockDriverBridge initialized. Audio will be sinked to /dev/null");
        return true;
    }
    
    void MockDriverBridge::Disconnect() { m_connected.store(false, std::memory_order_release); }
    
    bool MockDriverBridge::SendAudio(const Audio::AudioBuffer& buffer) noexcept {
        VC_PROFILE_ZONE("MockDriverBridge_SendAudio");
        // Simulate IO latency slightly if needed or just consume immediately
        return m_connected.load(std::memory_order_relaxed);
    }
}
