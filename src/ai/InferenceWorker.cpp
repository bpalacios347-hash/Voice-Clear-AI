/**
 * @file InferenceWorker.cpp
 * @brief Dedicated ONNX inference worker thread implementation.
 *
 * Operational contract:
 *   - The worker pops exactly FRAME_SAMPLES floats per iteration.
 *   - If the input ring has fewer samples, it sleeps for half a frame period
 *     (~5 ms) and retries — avoiding busy-waiting.
 *   - If inference throws, frames are passed through dry and the XRUN counter
 *     is incremented.
 *   - The MMCSS "Pro Audio" task class gives the thread a scheduling boost
 *     equivalent to what DAWs use, reducing OS-induced jitter.
 */

#include "InferenceWorker.h"
#include "../utils/Logger.h"
#include <windows.h>
#include <avrt.h>       // MMCSS
#include <chrono>
#include <algorithm>
#include <cassert>

#pragma comment(lib, "avrt.lib")

#include <timeapi.h>
#pragma comment(lib, "winmm.lib")

namespace VoiceClear::AI {

    // =========================================================================
    // Construction / Destruction
    // =========================================================================

    InferenceWorker::InferenceWorker(
        std::shared_ptr<Audio::LockFreeRingBuffer<float>> inputBuffer,
        std::shared_ptr<Audio::LockFreeRingBuffer<float>> outputBuffer,
        std::shared_ptr<Diagnostics::TelemetryManager>   telemetry)
        : m_inputBuffer(std::move(inputBuffer))
        , m_outputBuffer(std::move(outputBuffer))
        , m_telemetry(std::move(telemetry))
    {
        assert(m_inputBuffer  && "InferenceWorker: inputBuffer must not be null");
        assert(m_outputBuffer && "InferenceWorker: outputBuffer must not be null");
    }

    InferenceWorker::~InferenceWorker() {
        Stop();
    }

    // =========================================================================
    // Public API
    // =========================================================================

    void InferenceWorker::SetProcessor(std::unique_ptr<DeepFilterNetProcessor> processor) {
        assert(!m_running && "SetProcessor() called while worker is running");
        m_processor = std::move(processor);
    }

    bool InferenceWorker::Start() {
        if (m_running.exchange(true)) return true; // Already running

        if (!m_processor) {
            Utils::Logger::GetInstance()->Log(Core::LogLevel::Error,
                "[InferenceWorker] No processor set — cannot start.");
            m_running.store(false);
            return false;
        }

        m_wakeEvent = CreateEventA(nullptr, FALSE, FALSE, nullptr);

        m_workerThread = std::thread(&InferenceWorker::WorkerLoop, this);
        Utils::Logger::GetInstance()->Log(Core::LogLevel::Info,
            "[InferenceWorker] Dedicated ONNX inference thread started "
            "(frame=" + std::to_string(FRAME_SAMPLES) + " samples).");
        return true;
    }

    void InferenceWorker::Stop() {
        if (!m_running.exchange(false)) return;
        if (m_wakeEvent) {
            SetEvent(m_wakeEvent);
        }
        if (m_workerThread.joinable()) m_workerThread.join();
        if (m_wakeEvent) {
            CloseHandle(m_wakeEvent);
            m_wakeEvent = nullptr;
        }
        Utils::Logger::GetInstance()->Log(Core::LogLevel::Info,
            "[InferenceWorker] Worker thread stopped.");
    }

    void InferenceWorker::SetProfile(const NoiseProfile& profile) noexcept {
        if (m_processor) m_processor->SetProfile(profile);
    }

    size_t InferenceWorker::InputQueueDepth() const noexcept {
        return m_inputBuffer ? m_inputBuffer->ReadAvailable() : 0;
    }

    // =========================================================================
    // WorkerLoop
    // =========================================================================

    void InferenceWorker::WorkerLoop() {
        RegisterMmcss();
        timeBeginPeriod(1);

        // Statically allocated frame buffers — zero heap allocation in the hot path
        std::vector<float> frameBuf(FRAME_SAMPLES, 0.0f);

        while (m_running.load(std::memory_order_acquire)) {
            bool processedAny = false;

            // Drain all ready frames immediately to eliminate lag and prevent underruns
            while (m_inputBuffer->ReadAvailable() >= FRAME_SAMPLES && m_running.load(std::memory_order_relaxed)) {
                size_t popped = m_inputBuffer->Pop(frameBuf.data(), FRAME_SAMPLES);
                if (popped < FRAME_SAMPLES) break;

                processedAny = true;

                // ---- Build lightweight AudioBuffer view ----
                Audio::AudioBuffer buf;
                buf.samples    = std::span<float>(frameBuf.data(), FRAME_SAMPLES);
                buf.frames     = static_cast<uint32_t>(FRAME_SAMPLES);
                buf.channels   = 1;
                buf.sampleRate = 48000;

                // ---- Run ONNX inference ----
                auto t0 = std::chrono::high_resolution_clock::now();

                try {
                    m_processor->Process(buf);
                } catch (...) {
                    if (m_telemetry) m_telemetry->IncrementXrun();
                    Utils::Logger::GetInstance()->Log(Core::LogLevel::Error,
                        "[InferenceWorker] Inference exception — passing frame through dry.");
                }

                auto t1 = std::chrono::high_resolution_clock::now();
                float latMs = std::chrono::duration<float, std::milli>(t1 - t0).count();

                // ---- Update peak latency ----
                float prev = m_peakLatencyMs.load(std::memory_order_relaxed);
                while (latMs > prev &&
                       !m_peakLatencyMs.compare_exchange_weak(prev, latMs,
                           std::memory_order_relaxed)) {}

                if (m_telemetry) {
                    m_telemetry->PublishCpuUsage(latMs);
                    m_telemetry->PublishLatency(latMs);
                }

                // ---- Push processed frame to output ----
                m_outputBuffer->Push(frameBuf.data(), FRAME_SAMPLES);
            }

            if (!processedAny) {
                // Event-driven instant wakeup (<10 microseconds) when audio arrives, or 2ms timeout
                if (m_wakeEvent) {
                    WaitForSingleObject(m_wakeEvent, 2);
                } else {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
            }
        }

        timeEndPeriod(1);
        UnregisterMmcss();
    }

    // =========================================================================
    // MMCSS
    // =========================================================================

    void InferenceWorker::RegisterMmcss() noexcept {
        DWORD taskIndex = 0;
        m_mmcssTask = AvSetMmThreadCharacteristicsA("Pro Audio", &taskIndex);
        if (m_mmcssTask) {
            Utils::Logger::GetInstance()->Log(Core::LogLevel::Info,
                "[InferenceWorker] MMCSS 'Pro Audio' priority registered.");
        } else {
            Utils::Logger::GetInstance()->Log(Core::LogLevel::Warn,
                "[InferenceWorker] MMCSS registration failed (non-fatal). "
                "Error=" + std::to_string(static_cast<unsigned long>(GetLastError())));
        }
    }

    void InferenceWorker::UnregisterMmcss() noexcept {
        if (m_mmcssTask) {
            AvRevertMmThreadCharacteristics(m_mmcssTask);
            m_mmcssTask = nullptr;
        }
    }

} // namespace VoiceClear::AI
