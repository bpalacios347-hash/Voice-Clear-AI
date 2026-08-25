#include "AudioPipeline.h"
#include "../profiling/ProfilingMacros.h"
#include "../utils/Logger.h"
#include <algorithm>
#include <windows.h>
#include <chrono>

namespace VoiceClear::Audio {

    AudioPipeline::AudioPipeline(std::shared_ptr<Diagnostics::TelemetryManager> telemetry,
                                 std::shared_ptr<LockFreeRingBuffer<float>> inputBuffer,
                                 std::shared_ptr<LockFreeRingBuffer<float>> outputBuffer)
        : m_telemetry(std::move(telemetry)),
          m_inputBuffer(std::move(inputBuffer)),
          m_outputBuffer(std::move(outputBuffer)) {}

    AudioPipeline::~AudioPipeline() {
        Stop();
    }

    bool AudioPipeline::Start() {
        if (m_running.load()) return true;

        if (!m_inputBuffer || !m_outputBuffer) {
            Utils::Logger::GetInstance()->Log(Core::LogLevel::Error, "AudioPipeline missing ring buffers.");
            return false;
        }

        m_running.store(true);
        m_processingThread = std::thread(&AudioPipeline::ProcessingThreadLoop, this);
        
        // Elevate thread priority to near real-time (Pro Audio) equivalent
        SetThreadPriority(m_processingThread.native_handle(), THREAD_PRIORITY_HIGHEST);

        Utils::Logger::GetInstance()->Log(Core::LogLevel::Info, "AudioPipeline processing thread started.");
        return true;
    }

    void AudioPipeline::Stop() {
        if (!m_running.load()) return;
        m_running.store(false);
        if (m_processingThread.joinable()) {
            m_processingThread.join();
        }
        Utils::Logger::GetInstance()->Log(Core::LogLevel::Info, "AudioPipeline processing thread stopped.");
    }

    void AudioPipeline::SetAIProcessor(std::shared_ptr<IAudioStage> aiEngine) {
        m_aiEngine = std::move(aiEngine);
    }

    void AudioPipeline::EnableAI(bool enable) noexcept {
        m_aiEnabled.store(enable, std::memory_order_relaxed);
    }

    void AudioPipeline::AddStage(std::unique_ptr<IAudioStage> stage) {
        if (stage) {
            m_stages.push_back(std::move(stage));
        }
    }

    void AudioPipeline::ClearStages() {
        m_stages.clear();
    }

    void AudioPipeline::ProcessingThreadLoop() {
        std::vector<float> workBuffer(480, 0.0f); // 10ms at 48kHz
        AudioBuffer buffer;
        buffer.samples = workBuffer;
        buffer.frames = 480;
        buffer.channels = 1;
        buffer.sampleRate = 48000;

        while (m_running.load(std::memory_order_relaxed)) {
            // Check if input buffer has enough frames (480)
            if (m_inputBuffer->Occupancy() >= 480) {
                // Check if output buffer has enough space
                if ((m_outputBuffer->Capacity() - m_outputBuffer->Occupancy()) >= 480) {
                    
                    // Pop from input
                    m_inputBuffer->Pop(buffer.samples.data(), 480);
                    buffer.metadata.containsSpeech = false;

                    // Process DSP and AI
                    ProcessBlock(buffer);

                    // Push to output
                    m_outputBuffer->Push(buffer.samples.data(), 480);
                    
                } else {
                    // Output buffer is full (OutputEngine or DriverBridge is stalled)
                    if (m_telemetry) m_telemetry->IncrementXrun();
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
            } else {
                // Input buffer empty, wait for CaptureEngine to produce
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
    }

    void AudioPipeline::ProcessBlock(AudioBuffer& buffer) noexcept {
        VC_PROFILE_ZONE("AudioPipeline_ProcessBlock");
        
        auto start = std::chrono::high_resolution_clock::now();

        // 1. Standard DSP Stages (Format, Gain, VAD, etc.)
        for (const auto& stage : m_stages) {
            stage->Process(buffer);
        }

        // 2. AI Inference Engine (Synchronous injection)
        if (m_aiEnabled.load(std::memory_order_relaxed) && m_aiEngine) {
            m_aiEngine->Process(buffer);
        }

        auto end = std::chrono::high_resolution_clock::now();
        
        if (m_telemetry) {
            float durationMs = std::chrono::duration<float, std::milli>(end - start).count();
            // Publish the overall DSP pipeline duration
            m_telemetry->PublishCpuUsage(durationMs);
        }
    }

}
