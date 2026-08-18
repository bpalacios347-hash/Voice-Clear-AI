#pragma once
/**
 * @file InferenceWorker.h
 * @brief Decoupled ONNX inference worker thread.
 *
 * Problem solved:
 *   DeepFilterNetProcessor::Process() runs ONNX inference synchronously on
 *   the AudioPipeline capture thread.  If the ONNX runtime spikes > 15 ms
 *   (e.g. due to an OS scheduling hiccup) it stalls the capture ring buffer
 *   and causes audible XRUNs (dropouts).
 *
 * Solution:
 *   InferenceWorker owns a dedicated CPU thread that:
 *     1. Pops raw audio frames from an input LockFreeRingBuffer.
 *     2. Runs the ONNX inference (DeepFilterNetProcessor::Process).
 *     3. Pushes enhanced frames into an output LockFreeRingBuffer.
 *
 *   The AudioPipeline thread simply pushes/pops from these buffers with
 *   zero-blocking semantics.  If the inference falls behind, the pipeline
 *   performs a dry pass-through until the worker catches up.
 *
 * Thread-safety contract:
 *   - Push()/Pop() on LockFreeRingBuffer<float> are lock-free SPSC operations.
 *   - SetProcessor() MUST NOT be called while the worker is running.
 *   - SetProfile() is forwarded directly to DeepFilterNetProcessor which uses
 *     its own wait-free double-buffer protocol.
 */

#include <windows.h>
#include <memory>
#include <thread>
#include <atomic>
#include <span>
#include "IAudioProcessor.h"
#include "DeepFilterNetProcessor.h"
#include "NoiseProfile.h"
#include "../audio/LockFreeRingBuffer.h"
#include "../diagnostics/TelemetryManager.h"

namespace VoiceClear::AI {

    class InferenceWorker {
    public:
        /**
         * @param inputBuffer   SPSC ring fed by the capture/pipeline thread.
         * @param outputBuffer  SPSC ring drained by the output thread.
         * @param telemetry     Optional telemetry sink.
         */
        InferenceWorker(
            std::shared_ptr<Audio::LockFreeRingBuffer<float>> inputBuffer,
            std::shared_ptr<Audio::LockFreeRingBuffer<float>> outputBuffer,
            std::shared_ptr<Diagnostics::TelemetryManager>   telemetry = nullptr);

        ~InferenceWorker();

        // Non-copyable, non-movable
        InferenceWorker(const InferenceWorker&)            = delete;
        InferenceWorker& operator=(const InferenceWorker&) = delete;

        /**
         * @brief Inject the active DeepFilterNetProcessor.
         * Must be called BEFORE Start().
         */
        void SetProcessor(std::unique_ptr<DeepFilterNetProcessor> processor);

        /**
         * @brief Starts the worker thread and boosts it to Pro Audio priority via MMCSS.
         */
        bool Start();

        /**
         * @brief Signals the worker to drain and exit, then joins.
         */
        void Stop();

        /**
         * @brief Forwards a profile change to the underlying processor.
         * Lock-free: safe to call from any thread at any time.
         */
        void SetProfile(const NoiseProfile& profile) noexcept;

        /**
         * @brief Returns the number of frames currently waiting in the input buffer.
         */
        [[nodiscard]] size_t InputQueueDepth() const noexcept;

        /**
         * @brief Returns true if the worker is currently running.
         */
        [[nodiscard]] bool IsRunning() const noexcept {
            return m_running.load(std::memory_order_relaxed);
        }

        /**
         * @brief Returns the peak inference latency in milliseconds since last reset.
         */
        [[nodiscard]] float PeakInferenceLatencyMs() const noexcept {
            return m_peakLatencyMs.load(std::memory_order_relaxed);
        }

        /**
         * @brief Notifies the worker thread that new input samples have arrived.
         * Wakes up the thread immediately with sub-millisecond latency.
         */
        void NotifyAudioAvailable() noexcept {
            if (m_wakeEvent) {
                SetEvent(m_wakeEvent);
            }
        }

        /**
         * @brief Resets the peak latency counter.
         */
        void ResetLatencyStats() noexcept {
            m_peakLatencyMs.store(0.0f, std::memory_order_relaxed);
        }

    private:
        // ----------------------------------------------------------------
        // Members
        // ----------------------------------------------------------------
        std::shared_ptr<Audio::LockFreeRingBuffer<float>> m_inputBuffer;
        std::shared_ptr<Audio::LockFreeRingBuffer<float>> m_outputBuffer;
        std::shared_ptr<Diagnostics::TelemetryManager>   m_telemetry;

        std::unique_ptr<DeepFilterNetProcessor>  m_processor;

        std::atomic<bool>  m_running{false};
        std::thread        m_workerThread;

        std::atomic<float> m_peakLatencyMs{0.0f};

        // Win32 synchronization event for instant sub-ms wakeups
        HANDLE m_wakeEvent{nullptr};

        // MMCSS task handle (raised to Pro Audio priority)
        HANDLE m_mmcssTask{nullptr};

        // ----------------------------------------------------------------
        // Internal
        // ----------------------------------------------------------------
        void WorkerLoop();
        void RegisterMmcss() noexcept;
        void UnregisterMmcss() noexcept;

        // Frame size: 480 samples @ 48 kHz = exactly 10 ms
        static constexpr size_t FRAME_SAMPLES = 480;
    };

} // namespace VoiceClear::AI
