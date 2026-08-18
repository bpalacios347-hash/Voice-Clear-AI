#pragma once

// Common definitions shared between Kernel Mode (Driver) and User Mode (Service)
// MUST strictly use standard fixed-width types to guarantee ABI compatibility.

#ifdef _KERNEL_MODE
    #include <ntddk.h>
#else
    #include <atomic>
    #include <cstdint>
#endif

namespace VoiceClear { namespace Driver { namespace Protocol {

    constexpr UINT32 SHARED_PROTOCOL_VERSION = 1;
    constexpr UINT32 IOCTL_VOICECLEAR_CONNECT = 0x80002000; // FILE_DEVICE_VOICECLEAR = 0x8000, 0x800

    constexpr const char* VAD_DEVICE_PATH = "\\\\.\\VoiceClearVAD";
    constexpr const char* SHARED_MEM_NAME = "Local\\VoiceClear_VAD_SharedMem";

    // 48 kHz, Mono, Float32 is the canonical format for this system.
    constexpr UINT32 VAD_SAMPLE_RATE = 48000;
    constexpr UINT32 VAD_CHANNELS = 1;
    constexpr UINT32 VAD_BUFFER_CAPACITY_FRAMES = 48000; // 1 full second

    struct SharedMemoryHeader {
        UINT32 protocolVersion;
        UINT32 headerSize;

        UINT32 sampleRate;
        UINT32 channels;
        UINT32 capacity;

        // Atomic counters for SPSC lock-free ring buffer
#ifdef _KERNEL_MODE
        volatile LONG head; // Written by User Mode, Read by Kernel
        volatile LONG tail; // Read by User Mode, Read/Written by Kernel
#else
        std::atomic<UINT32> head;
        std::atomic<UINT32> tail;
#endif

        UINT64 timestamp;
        UINT64 sequenceNumber;
    };

    // The audio payload (float32 array of capacity length) follows immediately after this header.
    constexpr size_t CalculateSharedMemorySize() {
        return sizeof(SharedMemoryHeader) + (VAD_BUFFER_CAPACITY_FRAMES * sizeof(float));
    }

} } }
