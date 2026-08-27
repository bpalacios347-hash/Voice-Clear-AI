#include "OutputEngine.h"
#include "../utils/Logger.h"
#include "../profiling/ProfilingMacros.h"
#include <windows.h>
#include <mmdeviceapi.h>
#include <Audioclient.h>
#include <avrt.h>
#include <functiondiscoverykeys_devpkey.h>
#include <vector>
#include <cmath>
#include <algorithm>

namespace VoiceClear::Audio {

    OutputEngine::OutputEngine(std::shared_ptr<Diagnostics::TelemetryManager> telemetry)
        : m_telemetry(std::move(telemetry)) {}

    OutputEngine::~OutputEngine() {
        Shutdown();
    }

    bool OutputEngine::Initialize(const OutputConfig& config) {
        m_deviceId = config.deviceId;
        m_isExclusive = false;
        m_initialized = true;
        return true;
    }

    bool OutputEngine::Start() {
        if (!m_initialized || m_running) return false;

        m_running = true;
        m_renderThread = std::thread(&OutputEngine::RenderThreadLoop, this);
        return true;
    }

    void OutputEngine::Stop() {
        if (!m_running) return;
        m_running = false;

        if (m_renderThread.joinable()) {
            m_renderThread.join();
        }
    }

    void OutputEngine::Shutdown() {
        Stop();
        m_initialized = false;
    }

static void WriteAudioToWasapiBuffer(
    BYTE* pDst,
    const float* pSrc,
    size_t framesToWrite,
    int srcChannels,
    int dstChannels,
    const WAVEFORMATEX* pWfx)
{
    if (!pDst || !pSrc || framesToWrite == 0 || !pWfx || dstChannels <= 0) return;

    bool isFloat = (pWfx->wFormatTag == WAVE_FORMAT_IEEE_FLOAT);
    bool isExt = (pWfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE);
    const WAVEFORMATEXTENSIBLE* pExt = isExt ? reinterpret_cast<const WAVEFORMATEXTENSIBLE*>(pWfx) : nullptr;

    if (isExt && pExt->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT) {
        isFloat = true;
    }

    if (isFloat) {
        float* dstFloat = reinterpret_cast<float*>(pDst);
        for (size_t i = 0; i < framesToWrite; ++i) {
            for (int c = 0; c < dstChannels; ++c) {
                int srcC = (c < srcChannels) ? c : 0;
                dstFloat[i * dstChannels + c] = pSrc[i * srcChannels + srcC];
            }
        }
        return;
    }

    if (pWfx->wBitsPerSample == 16) {
        int16_t* dst16 = reinterpret_cast<int16_t*>(pDst);
        for (size_t i = 0; i < framesToWrite; ++i) {
            for (int c = 0; c < dstChannels; ++c) {
                int srcC = (c < srcChannels) ? c : 0;
                float s = std::clamp(pSrc[i * srcChannels + srcC], -1.0f, 1.0f);
                dst16[i * dstChannels + c] = static_cast<int16_t>(s * 32767.0f);
            }
        }
    } else if (pWfx->wBitsPerSample == 32) {
        int32_t* dst32 = reinterpret_cast<int32_t*>(pDst);
        bool is24BitIn32 = isExt && (pExt->Samples.wValidBitsPerSample == 24);
        for (size_t i = 0; i < framesToWrite; ++i) {
            for (int c = 0; c < dstChannels; ++c) {
                int srcC = (c < srcChannels) ? c : 0;
                float s = std::clamp(pSrc[i * srcChannels + srcC], -1.0f, 1.0f);
                if (is24BitIn32) {
                    int32_t val24 = static_cast<int32_t>(s * 8388607.0f);
                    dst32[i * dstChannels + c] = val24 << 8;
                } else {
                    dst32[i * dstChannels + c] = static_cast<int32_t>(s * 2147483647.0f);
                }
            }
        }
    } else if (pWfx->wBitsPerSample == 24) {
        uint8_t* dst24 = pDst;
        for (size_t i = 0; i < framesToWrite; ++i) {
            for (int c = 0; c < dstChannels; ++c) {
                int srcC = (c < srcChannels) ? c : 0;
                float s = std::clamp(pSrc[i * srcChannels + srcC], -1.0f, 1.0f);
                int32_t val = static_cast<int32_t>(s * 8388607.0f);
                size_t byteIdx = (i * dstChannels + c) * 3;
                dst24[byteIdx]     = static_cast<uint8_t>(val & 0xFF);
                dst24[byteIdx + 1] = static_cast<uint8_t>((val >> 8) & 0xFF);
                dst24[byteIdx + 2] = static_cast<uint8_t>((val >> 16) & 0xFF);
            }
        }
    } else {
        std::memset(pDst, 0, framesToWrite * pWfx->nBlockAlign);
    }
}

    void OutputEngine::RenderThreadLoop() {
        VC_PROFILE_ZONE("RenderThreadLoop");

        HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
        DWORD taskIndex = 0;
        HANDLE hTask = AvSetMmThreadCharacteristicsW(L"Pro Audio", &taskIndex);

        auto logger = Utils::Logger::GetInstance();
        logger->Log(Core::LogLevel::Info, "[OutputEngine] Render thread started for: " + m_deviceId);

        IMMDeviceEnumerator* pEnumerator = nullptr;
        IMMDevice* pDevice = nullptr;
        IAudioClient* pAudioClient = nullptr;
        IAudioRenderClient* pRenderClient = nullptr;
        HANDLE hRenderEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        WAVEFORMATEX* pWfx = nullptr;
        UINT32 bufferFrameCount = 0;

        // Optional monitor client (default speakers/headphones)
        IMMDevice* pMonDevice = nullptr;
        IAudioClient* pMonAudioClient = nullptr;
        IAudioRenderClient* pMonRenderClient = nullptr;
        WAVEFORMATEX* pMonWfx = nullptr;
        UINT32 monBufferFrameCount = 0;

        hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL,
                              __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);

        if (SUCCEEDED(hr) && pEnumerator) {
            // Find target render device
            if (m_deviceId.empty() || m_deviceId == "default") {
                // Search specifically for CABLE Input (VB-Audio Virtual Cable)
                IMMDeviceCollection* pCollection = nullptr;
                if (SUCCEEDED(pEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pCollection)) && pCollection) {
                    UINT count = 0;
                    pCollection->GetCount(&count);
                    for (UINT i = 0; i < count; ++i) {
                        IMMDevice* pCandidate = nullptr;
                        if (SUCCEEDED(pCollection->Item(i, &pCandidate)) && pCandidate) {
                            IPropertyStore* pProps = nullptr;
                            if (SUCCEEDED(pCandidate->OpenPropertyStore(STGM_READ, &pProps)) && pProps) {
                                PROPVARIANT varName;
                                PropVariantInit(&varName);
                                if (SUCCEEDED(pProps->GetValue(PKEY_Device_FriendlyName, &varName)) && varName.pwszVal) {
                                    std::wstring wname(varName.pwszVal);
                                    if (wname.find(L"CABLE") != std::wstring::npos || wname.find(L"Cable") != std::wstring::npos) {
                                        pDevice = pCandidate;
                                        pDevice->AddRef();
                                        PropVariantClear(&varName);
                                        pProps->Release();
                                        pCandidate->Release();
                                        break;
                                    }
                                }
                                PropVariantClear(&varName);
                                pProps->Release();
                            }
                            pCandidate->Release();
                        }
                    }
                    pCollection->Release();
                }

                if (!pDevice) {
                    logger->Log(Core::LogLevel::Warn, "[OutputEngine] VB-Cable not detected as default output target.");
                }
            } else {
                int wlen = MultiByteToWideChar(CP_UTF8, 0, m_deviceId.c_str(), -1, NULL, 0);
                std::vector<wchar_t> wDevId(wlen);
                MultiByteToWideChar(CP_UTF8, 0, m_deviceId.c_str(), -1, wDevId.data(), wlen);
                hr = pEnumerator->GetDevice(wDevId.data(), &pDevice);
            }

            // Also get default playback endpoint for "Hear Myself" monitor mirroring (Headphones/Speakers)
            pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pMonDevice);
        }

        if (pDevice) {
            hr = pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&pAudioClient);
        }

        if (SUCCEEDED(hr) && pAudioClient) {
            hr = pAudioClient->GetMixFormat(&pWfx);
        }

        if (SUCCEEDED(hr) && pAudioClient && pWfx) {
            m_sampleRate = pWfx->nSamplesPerSec;
            m_channels   = pWfx->nChannels;

            REFERENCE_TIME bufferDuration = 200000; // 20 ms buffer
            hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,
                                         AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
                                         bufferDuration, 0, pWfx, NULL);

            if (SUCCEEDED(hr)) {
                pAudioClient->GetBufferSize(&bufferFrameCount);
                pAudioClient->SetEventHandle(hRenderEvent);
                pAudioClient->GetService(__uuidof(IAudioRenderClient), (void**)&pRenderClient);

                // Pre-fill with silence before starting to avoid startup underrun
                BYTE* pInitial = nullptr;
                if (SUCCEEDED(pRenderClient->GetBuffer(bufferFrameCount, &pInitial))) {
                    std::memset(pInitial, 0, bufferFrameCount * pWfx->nBlockAlign);
                    pRenderClient->ReleaseBuffer(bufferFrameCount, 0);
                }

                pAudioClient->Start();
                logger->Log(Core::LogLevel::Info, "[OutputEngine] Primary WASAPI render active: " +
                    std::to_string(m_sampleRate) + "Hz, " + std::to_string(m_channels) + "ch");
            }
        }

        // Initialize secondary monitor client (Hear Myself mode)
        if (pMonDevice) {
            if (SUCCEEDED(pMonDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&pMonAudioClient))) {
                if (SUCCEEDED(pMonAudioClient->GetMixFormat(&pMonWfx))) {
                    REFERENCE_TIME monDur = 200000;
                    if (SUCCEEDED(pMonAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, monDur, 0, pMonWfx, NULL))) {
                        pMonAudioClient->GetBufferSize(&monBufferFrameCount);
                        pMonAudioClient->GetService(__uuidof(IAudioRenderClient), (void**)&pMonRenderClient);

                        BYTE* pInitMon = nullptr;
                        if (SUCCEEDED(pMonRenderClient->GetBuffer(monBufferFrameCount, &pInitMon))) {
                            std::memset(pInitMon, 0, monBufferFrameCount * pMonWfx->nBlockAlign);
                            pMonRenderClient->ReleaseBuffer(monBufferFrameCount, 0);
                        }
                        pMonAudioClient->Start();
                    }
                }
            }
        }

        m_monitorBuffer = std::make_unique<LockFreeRingBuffer<float>>(48000 * 2);

        std::vector<float> audioSamples;
        audioSamples.reserve(4096);
        std::vector<float> monSamples;
        monSamples.reserve(4096);

        while (m_running.load(std::memory_order_relaxed)) {
            DWORD waitRes = WaitForSingleObject(hRenderEvent, 20);

            UINT32 framesNeeded = 0;
            if (pAudioClient && pRenderClient && pWfx) {
                UINT32 padding = 0;
                if (SUCCEEDED(pAudioClient->GetCurrentPadding(&padding))) {
                    framesNeeded = bufferFrameCount - padding;
                }
            }

            UINT32 monNeeded = 0;
            bool isMonActive = m_monitorEnabled.load(std::memory_order_relaxed) && pMonRenderClient && pMonAudioClient && pMonWfx;
            if (isMonActive) {
                UINT32 monPadding = 0;
                if (SUCCEEDED(pMonAudioClient->GetCurrentPadding(&monPadding))) {
                    monNeeded = monBufferFrameCount - monPadding;
                }
            }

            // Fetch from pipeline if primary output needs data (or if monitor is active and primary is inactive)
            UINT32 fetchFrames = framesNeeded;
            if (fetchFrames == 0 && isMonActive && (!pAudioClient || !pRenderClient)) {
                fetchFrames = monNeeded;
            }

            if (fetchFrames > 0) {
                audioSamples.resize(fetchFrames * m_channels);
                size_t got = 0;

                if (m_callback) {
                    got = m_callback(audioSamples.data(), fetchFrames, m_channels, m_sampleRate);
                }

                if (got < fetchFrames) {
                    if (m_telemetry) m_telemetry->IncrementXrun();
                    std::fill(audioSamples.begin() + (got * m_channels), audioSamples.end(), 0.0f);
                }

                // If monitor is active, copy clean audio stream to monitor buffer
                if (isMonActive && m_monitorBuffer) {
                    m_monitorBuffer->Push(audioSamples.data(), fetchFrames * m_channels);
                }

                // Calculate Output RMS level for live VU meter
                float sumSq = 0.0f;
                for (size_t i = 0; i < got * m_channels; ++i) {
                    sumSq += audioSamples[i] * audioSamples[i];
                }
                float rms = std::sqrt(sumSq / std::max(1.0f, static_cast<float>(got * m_channels)));
                if (m_telemetry) {
                    m_telemetry->PublishOutputLevel(std::min(1.0f, rms * 2.5f));
                }

                // Write to primary render target (CABLE Input)
                if (pRenderClient && pWfx && framesNeeded > 0) {
                    BYTE* pData = nullptr;
                    if (SUCCEEDED(pRenderClient->GetBuffer(fetchFrames, &pData))) {
                        WriteAudioToWasapiBuffer(pData, audioSamples.data(), fetchFrames, m_channels, pWfx->nChannels, pWfx);
                        pRenderClient->ReleaseBuffer(fetchFrames, 0);
                    }
                }
            }

            // Write to monitor headphones from the dedicated monitor FIFO
            if (isMonActive && monNeeded > 0 && m_monitorBuffer) {
                // Prevent monitor clock drift from accumulating delay (>70ms)
                size_t avail = m_monitorBuffer->ReadAvailable();
                size_t maxAllowed = (m_sampleRate * m_channels * 70) / 1000;
                if (avail > maxAllowed) {
                    size_t toDrop = avail - (m_sampleRate * m_channels * 35) / 1000;
                    std::vector<float> dropBuf(toDrop);
                    m_monitorBuffer->Pop(dropBuf.data(), toDrop);
                }

                monSamples.resize(monNeeded * m_channels);
                size_t popped = m_monitorBuffer->Pop(monSamples.data(), monNeeded * m_channels);
                if (popped < monNeeded * m_channels) {
                    if (popped > 0) {
                        for (size_t i = popped; i < monNeeded * m_channels; ++i) {
                            monSamples[i] = monSamples[popped - 1] * std::pow(0.92f, static_cast<float>(i - popped + 1));
                        }
                    } else {
                        std::fill(monSamples.begin(), monSamples.end(), 0.0f);
                    }
                }

                BYTE* pMonData = nullptr;
                if (SUCCEEDED(pMonRenderClient->GetBuffer(monNeeded, &pMonData))) {
                    WriteAudioToWasapiBuffer(pMonData, monSamples.data(), monNeeded, m_channels, pMonWfx->nChannels, pMonWfx);
                    pMonRenderClient->ReleaseBuffer(monNeeded, 0);
                }
            }

            if (framesNeeded == 0 && (!isMonActive || monNeeded == 0)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
            }
        }

        if (pMonAudioClient) { pMonAudioClient->Stop(); pMonAudioClient->Release(); }
        if (pMonRenderClient) pMonRenderClient->Release();
        if (pMonDevice) pMonDevice->Release();
        if (pMonWfx) CoTaskMemFree(pMonWfx);

        if (pAudioClient) { pAudioClient->Stop(); pAudioClient->Release(); }
        if (pRenderClient) pRenderClient->Release();
        if (pDevice) pDevice->Release();
        if (pEnumerator) pEnumerator->Release();
        if (pWfx) CoTaskMemFree(pWfx);
        if (hRenderEvent) CloseHandle(hRenderEvent);

        if (hTask) AvRevertMmThreadCharacteristics(hTask);
        CoUninitialize();
        logger->Log(Core::LogLevel::Info, "[OutputEngine] Render thread exited.");
    }

}
