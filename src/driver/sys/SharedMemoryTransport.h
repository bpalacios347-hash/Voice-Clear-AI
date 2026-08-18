#pragma once
#include <ntddk.h>
#include "../common/VoiceClearSharedProtocol.h"

namespace VoiceClear { namespace Driver { namespace Kernel {

    /**
     * @brief Manages mapping and consuming the shared memory ring buffer from User Mode.
     */
    class SharedMemoryTransport {
    public:
        SharedMemoryTransport() = default;
        ~SharedMemoryTransport() { Disconnect(); }

        NTSTATUS Connect() {
            if (m_connected) return STATUS_SUCCESS;

            UNICODE_STRING sectionName;
            RtlInitUnicodeString(&sectionName, L"\\BaseNamedObjects\\Local\\VoiceClear_VAD_SharedMem");

            OBJECT_ATTRIBUTES objAttr;
            InitializeObjectAttributes(&objAttr, &sectionName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

            NTSTATUS status = ZwOpenSection(&m_hSection, SECTION_MAP_READ, &objAttr);
            if (!NT_SUCCESS(status)) {
                return status;
            }

            SIZE_T viewSize = Protocol::CalculateSharedMemorySize();
            status = ZwMapViewOfSection(
                m_hSection,
                NtCurrentProcess(),
                &m_pSharedMemoryView,
                0,
                viewSize,
                NULL,
                &viewSize,
                ViewUnmap,
                0,
                PAGE_READWRITE
            );

            if (!NT_SUCCESS(status)) {
                ZwClose(m_hSection);
                m_hSection = NULL;
                return status;
            }

            m_pHeader = reinterpret_cast<Protocol::SharedMemoryHeader*>(m_pSharedMemoryView);
            m_pPayload = reinterpret_cast<float*>(reinterpret_cast<UINT8*>(m_pSharedMemoryView) + sizeof(Protocol::SharedMemoryHeader));

            // Validate protocol version
            if (m_pHeader->protocolVersion != Protocol::SHARED_PROTOCOL_VERSION) {
                Disconnect();
                return STATUS_REVISION_MISMATCH;
            }

            m_connected = true;
            return STATUS_SUCCESS;
        }

        void Disconnect() {
            if (m_pSharedMemoryView) {
                ZwUnmapViewOfSection(NtCurrentProcess(), m_pSharedMemoryView);
                m_pSharedMemoryView = nullptr;
                m_pHeader = nullptr;
                m_pPayload = nullptr;
            }
            if (m_hSection) {
                ZwClose(m_hSection);
                m_hSection = NULL;
            }
            m_connected = false;
        }

        bool IsConnected() const { return m_connected; }

        /**
         * @brief Non-blocking read. Returns frames read.
         */
        UINT32 Read(float* outBuffer, UINT32 requestedFrames) {
            if (!m_connected || !m_pHeader) return 0;

            // Strict atomic barriers
            LONG head = InterlockedCompareExchange(&m_pHeader->head, 0, 0);
            LONG tail = InterlockedCompareExchange(&m_pHeader->tail, 0, 0);

            UINT32 capacity = m_pHeader->capacity;
            if (capacity == 0) return 0;

            UINT32 available = (head >= tail) ? (head - tail) : (capacity - tail + head);
            UINT32 toRead = (requestedFrames < available) ? requestedFrames : available;

            if (toRead == 0) return 0;

            UINT32 firstChunk = (capacity - tail < toRead) ? (capacity - tail) : toRead;
            RtlCopyMemory(outBuffer, m_pPayload + tail, firstChunk * sizeof(float));

            if (firstChunk < toRead) {
                RtlCopyMemory(outBuffer + firstChunk, m_pPayload, (toRead - firstChunk) * sizeof(float));
            }

            InterlockedExchange(&m_pHeader->tail, (tail + toRead) % capacity);

            return toRead;
        }

    private:
        bool m_connected = false;
        HANDLE m_hSection = NULL;
        PVOID m_pSharedMemoryView = nullptr;
        Protocol::SharedMemoryHeader* m_pHeader = nullptr;
        float* m_pPayload = nullptr;
    };

} } }
