#include "Driver.h"
#include "SharedMemoryTransport.h"

using namespace VoiceClear::Driver::Kernel;

const GUID KSNODENAME_VOICECLEAR_MIC = {0x91234567, 0x89AB, 0xCDEF, {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF}};

// Pin Context to hold our transport instance per-stream
struct KSPIN_CONTEXT {
    SharedMemoryTransport* Transport;
};

NTSTATUS PinCreate(PKSPIN Pin, PIRP Irp) {
    auto Context = new (NonPagedPool, POOLTAG_VOICECLEAR) KSPIN_CONTEXT;
    if (!Context) return STATUS_INSUFFICIENT_RESOURCES;

    Context->Transport = new (NonPagedPool, POOLTAG_VOICECLEAR) SharedMemoryTransport();
    if (!Context->Transport) {
        delete Context;
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    // Try to connect. If the service isn't running, it will fail, but we'll accept it.
    // The driver should output digital silence in that case.
    Context->Transport->Connect();
    
    Pin->Context = Context;
    return STATUS_SUCCESS;
}

NTSTATUS PinClose(PKSPIN Pin, PIRP Irp) {
    auto Context = static_cast<KSPIN_CONTEXT*>(Pin->Context);
    if (Context) {
        if (Context->Transport) {
            delete Context->Transport;
        }
        delete Context;
    }
    return STATUS_SUCCESS;
}

NTSTATUS PinProcess(PKSPIN Pin) {
    auto Context = static_cast<KSPIN_CONTEXT*>(Pin->Context);
    if (!Context) return STATUS_UNSUCCESSFUL;

    PKSSTREAM_POINTER leading = KsPinGetLeadingEdgeStreamPointer(Pin, KSSTREAM_POINTER_STATE_LOCKED);
    if (!leading) return STATUS_SUCCESS;

    PVOID buffer = leading->OffsetOut.Data;
    ULONG byteCount = leading->OffsetOut.Count;
    
    // We advertise 16-bit PCM Mono
    UINT32 requestedFrames = byteCount / sizeof(INT16);
    
    if (Context->Transport && Context->Transport->IsConnected()) {
        // We need a temporary float buffer to read from the service
        // Since we are in kernel mode, allocating a large buffer on the stack is bad,
        // so we allocate from pool or just read chunk by chunk.
        // For simplicity, allocate a temp buffer
        float* tempFloatBuffer = (float*)ExAllocatePoolWithTag(NonPagedPool, requestedFrames * sizeof(float), POOLTAG_VOICECLEAR);
        if (tempFloatBuffer) {
            UINT32 framesRead = Context->Transport->Read(tempFloatBuffer, requestedFrames);
            
            // Convert to 16-bit PCM
            INT16* pcmBuffer = static_cast<INT16*>(buffer);
            for (UINT32 i = 0; i < framesRead; i++) {
                float sample = tempFloatBuffer[i];
                if (sample > 1.0f) sample = 1.0f;
                if (sample < -1.0f) sample = -1.0f;
                pcmBuffer[i] = static_cast<INT16>(sample * 32767.0f);
            }
            
            // Zero-fill underrun / dropped frames
            if (framesRead < requestedFrames) {
                UINT32 missingFrames = requestedFrames - framesRead;
                RtlZeroMemory(pcmBuffer + framesRead, missingFrames * sizeof(INT16));
            }
            ExFreePoolWithTag(tempFloatBuffer, POOLTAG_VOICECLEAR);
        } else {
            RtlZeroMemory(buffer, byteCount); // Failed to allocate temp buffer
        }
    } else {
        // Service not running: digital silence
        RtlZeroMemory(buffer, byteCount);
        
        // Attempt reconnect lazily for next time
        if (Context->Transport) {
            Context->Transport->Connect();
        }
    }

    KsStreamPointerUnlock(leading, TRUE);
    return STATUS_SUCCESS;
}

const KSPIN_DISPATCH PinDispatch = {
    PinCreate,
    PinClose,
    PinProcess,
    nullptr, // Reset
    nullptr, // SetDataFormat
    nullptr, // SetDeviceState
    nullptr, // Connect
    nullptr, // Disconnect
    nullptr, // Clock
    nullptr  // Allocator
};

const KSDATARANGE_AUDIO PinDataFormat = {
    {
        sizeof(KSDATARANGE_AUDIO),
        0,
        0,
        0,
        {0x73647561L, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}}, // STATIC_KSDATAFORMAT_TYPE_AUDIO
        {0x00000001L, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}}, // STATIC_KSDATAFORMAT_SUBTYPE_PCM
        {0x05589f81L, 0xc356, 0x11ce, {0xbf, 0x01, 0x00, 0xaa, 0x00, 0x55, 0x59, 0x5a}}  // STATIC_KSDATAFORMAT_SPECIFIER_WAVEFORMATEX
    },
    1, // MaximumChannels
    16, // MinimumBitsPerSample
    16, // MaximumBitsPerSample
    48000, // MinimumSampleFrequency
    48000 // MaximumSampleFrequency
};

const PKSDATAFORMAT PinDataRanges[] = {
    (PKSDATAFORMAT)&PinDataFormat
};

const KSDATARANGE BridgePinDataFormat = {
    sizeof(KSDATARANGE),
    0,
    0,
    0,
    {0x73647561L, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}}, // KSDATAFORMAT_TYPE_AUDIO
    {0x1dbaec64L, 0xf6be, 0x464f, {0xb3, 0xf7, 0x76, 0xa5, 0x6f, 0x56, 0xbe, 0xc1}}, // KSDATAFORMAT_SUBTYPE_ANALOG
    {0x0F6417D6L, 0xC318, 0x11D0, {0xA4, 0x3F, 0x00, 0xA0, 0xC9, 0x22, 0x31, 0x96}}  // KSDATAFORMAT_SPECIFIER_NONE
};

const PKSDATAFORMAT BridgePinDataRanges[] = {
    (PKSDATAFORMAT)&BridgePinDataFormat
};

extern const KSPIN_DESCRIPTOR_EX PinDescriptors[] = {
    {
        &PinDispatch,
        nullptr, // Automation Table
        {
            0, // Interfaces Count
            nullptr,
            0, // Mediums Count
            nullptr,
            1, // Data Ranges Count
            PinDataRanges,
            KSPIN_DATAFLOW_OUT,
            KSPIN_COMMUNICATION_BOTH,
            (GUID*)&KSCATEGORY_AUDIO,
            (GUID*)&PINNAME_CAPTURE,
            0
        },
        0, // Flags
        1, // Instances Possible
        1, // Instances Necessary
        nullptr,
        nullptr
    },
    { // Pin 1 - Bridge Pin (Microphone Physical connection)
        nullptr, // No dispatch (bridge pin doesn't process data)
        nullptr, // Automation Table
        {
            0, // Interfaces Count
            nullptr,
            0, // Mediums Count
            nullptr,
            1, // Data Ranges Count
            BridgePinDataRanges,
            KSPIN_DATAFLOW_IN,
            KSPIN_COMMUNICATION_NONE,
            (GUID*)&KSNODETYPE_MICROPHONE,
            (GUID*)&KSNODENAME_VOICECLEAR_MIC,
            0
        },
        0, // Flags
        1, // Instances Possible
        1, // Instances Necessary
        nullptr,
        nullptr
    }
};

extern const ULONG PinDescriptorCount = ARRAYSIZE(PinDescriptors);
