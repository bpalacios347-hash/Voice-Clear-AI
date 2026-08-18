#include "CaptureEngine.h"
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

    CaptureEngine::CaptureEngine(std::shared_ptr<Diagnostics::TelemetryManager> telemetry)
        : m_telemetry(std::move(telemetry)) {}

    CaptureEngine::~CaptureEngine() {
        Shutdown();
    }

    bool CaptureEngine::Initialize(const CaptureConfig& config) {
        m_deviceId = config.deviceId;
        m_isExclusive = false;
        m_initialized = true;
        return true;
    }

    bool CaptureEngine::Start() {
        if (!m_initialized || m_running) return false;

        m_running = true;
        m_captureThread = std::thread(&CaptureEngine::CaptureThreadLoop, this);
        return true;
    }

    void CaptureEngine::Stop() {
        if (!m_running) return;
        m_running = false;

        if (m_captureThread.joinable()) {
            m_captureThread.join();
        }
    }

    void CaptureEngine::Shutdown() {
        Stop();
        m_initialized = false;
    }

    void CaptureEngine::CaptureThreadLoop() {
        VC_PROFILE_ZONE("CaptureThreadLoop");

        HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
        DWORD taskIndex = 0;
        HANDLE hTask = AvSetMmThreadCharacteristicsW(L"Pro Audio", &taskIndex);

        auto logger = Utils::Logger::GetInstance();
        logger->Log(Core::LogLevel::Info, "[CaptureEngine] Thread started for device: " + m_deviceId);

        IMMDeviceEnumerator* pEnumerator = nullptr;
        IMMDevice* pDevice = nullptr;
        IAudioClient* pAudioClient = nullptr;
        IAudioCaptureClient* pCaptureClient = nullptr;
        HANDLE hCaptureEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        WAVEFORMATEX* pWfx = nullptr;

        hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL,
                              __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);

        if (SUCCEEDED(hr) && pEnumerator) {
            if (m_deviceId.empty() || m_deviceId == "default") {
                // Find default physical microphone (skip VB-Cable/loopback to prevent infinite feedback)
                IMMDeviceCollection* pCollection = nullptr;
                if (SUCCEEDED(pEnumerator->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE, &pCollection)) && pCollection) {
                    UINT count = 0;
                    pCollection->GetCount(&count);
                    IMMDevice* pFallback = nullptr;
                    for (UINT i = 0; i < count; ++i) {
                        IMMDevice* pCandidate = nullptr;
                        if (SUCCEEDED(pCollection->Item(i, &pCandidate)) && pCandidate) {
                            IPropertyStore* pProps = nullptr;
                            std::wstring friendlyName;
                            if (SUCCEEDED(pCandidate->OpenPropertyStore(STGM_READ, &pProps)) && pProps) {
                                PROPVARIANT varName;
                                PropVariantInit(&varName);
                                if (SUCCEEDED(pProps->GetValue(PKEY_Device_FriendlyName, &varName)) && varName.pwszVal) {
                                    friendlyName = varName.pwszVal;
                                }
                                PropVariantClear(&varName);
                                pProps->Release();
                            }

                            bool isVirtual = (friendlyName.find(L"CABLE") != std::wstring::npos ||
                                              friendlyName.find(L"Cable") != std::wstring::npos ||
                                              friendlyName.find(L"VB-Audio") != std::wstring::npos ||
                                              friendlyName.find(L"Stereo Mix") != std::wstring::npos ||
                                              friendlyName.find(L"Mezcla") != std::wstring::npos);

                            if (!isVirtual && !pDevice) {
                                pDevice = pCandidate;
                                pDevice->AddRef();
                            } else if (!pFallback) {
                                pFallback = pCandidate;
                                pFallback->AddRef();
                            }
                            pCandidate->Release();
                        }
                    }
                    if (!pDevice && pFallback) {
                        pDevice = pFallback;
                    } else if (pFallback) {
                        pFallback->Release();
                    }
                    pCollection->Release();
                }
                if (!pDevice) {
                    pEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &pDevice);
                }
            } else {
                int wlen = MultiByteToWideChar(CP_UTF8, 0, m_deviceId.c_str(), -1, NULL, 0);
                std::vector<wchar_t> wDevId(wlen);
                MultiByteToWideChar(CP_UTF8, 0, m_deviceId.c_str(), -1, wDevId.data(), wlen);
                hr = pEnumerator->GetDevice(wDevId.data(), &pDevice);
                
                // If GetDevice fails by exact string, search endpoints by ID or Friendly Name
                if (FAILED(hr) || !pDevice) {
                    IMMDeviceCollection* pCollection = nullptr;
                    if (SUCCEEDED(pEnumerator->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE, &pCollection)) && pCollection) {
                        UINT count = 0;
                        pCollection->GetCount(&count);
                        for (UINT i = 0; i < count; ++i) {
                            IMMDevice* pCandidate = nullptr;
                            if (SUCCEEDED(pCollection->Item(i, &pCandidate)) && pCandidate) {
                                LPWSTR pwszId = nullptr;
                                pCandidate->GetId(&pwszId);

                                IPropertyStore* pProps = nullptr;
                                std::wstring friendlyName;
                                if (SUCCEEDED(pCandidate->OpenPropertyStore(STGM_READ, &pProps)) && pProps) {
                                    PROPVARIANT varName;
                                    PropVariantInit(&varName);
                                    if (SUCCEEDED(pProps->GetValue(PKEY_Device_FriendlyName, &varName)) && varName.pwszVal) {
                                        friendlyName = varName.pwszVal;
                                    }
                                    PropVariantClear(&varName);
                                    pProps->Release();
                                }

                                bool matched = false;
                                if (pwszId && wDevId.data() && (wcsstr(pwszId, wDevId.data()) != nullptr || wcsstr(wDevId.data(), pwszId) != nullptr)) {
                                    matched = true;
                                }
                                if (!friendlyName.empty() && wDevId.data() && (friendlyName.find(wDevId.data()) != std::wstring::npos || wcsstr(wDevId.data(), friendlyName.c_str()) != nullptr)) {
                                    matched = true;
                                }

                                if (pwszId) CoTaskMemFree(pwszId);

                                if (matched) {
                                    pDevice = pCandidate;
                                    pDevice->AddRef();
                                    pCandidate->Release();
                                    logger->Log(Core::LogLevel::Info, "[CaptureEngine] Matched capture device by search.");
                                    break;
                                }
                                pCandidate->Release();
                            }
                        }
                        pCollection->Release();
                    }
                }

                if (!pDevice) {
                    pEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &pDevice);
                    logger->Log(Core::LogLevel::Warn, "[CaptureEngine] Target device not found; fell back to default capture endpoint.");
                }
            }
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
                pAudioClient->SetEventHandle(hCaptureEvent);
                pAudioClient->GetService(__uuidof(IAudioCaptureClient), (void**)&pCaptureClient);
                pAudioClient->Start();
                logger->Log(Core::LogLevel::Info, "[CaptureEngine] WASAPI active streaming: " +
                    std::to_string(m_sampleRate) + "Hz, " + std::to_string(m_channels) + "ch");
            }
        }

        std::vector<float> convertedSamples;
        convertedSamples.reserve(4096);

        while (m_running.load(std::memory_order_relaxed)) {
            DWORD waitRes = WaitForSingleObject(hCaptureEvent, 20);

            if (pCaptureClient) {
                UINT32 packetLength = 0;
                hr = pCaptureClient->GetNextPacketSize(&packetLength);

                while (SUCCEEDED(hr) && packetLength > 0 && m_running.load(std::memory_order_relaxed)) {
                    BYTE* pData = nullptr;
                    UINT32 numFramesRead = 0;
                    DWORD flags = 0;

                    hr = pCaptureClient->GetBuffer(&pData, &numFramesRead, &flags, NULL, NULL);
                    if (FAILED(hr)) break;

                    if (numFramesRead > 0 && pWfx) {
                        convertedSamples.resize(numFramesRead * m_channels);

                        if (flags & AUDCLNT_BUFFERFLAGS_SILENT) {
                            std::fill(convertedSamples.begin(), convertedSamples.end(), 0.0f);
                        } else {
                            bool isFloat = (pWfx->wFormatTag == WAVE_FORMAT_IEEE_FLOAT);
                            bool isExt = (pWfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE);
                            const WAVEFORMATEXTENSIBLE* pExt = isExt ? reinterpret_cast<const WAVEFORMATEXTENSIBLE*>(pWfx) : nullptr;
                            if (isExt && pExt->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT) {
                                isFloat = true;
                            }

                            if (isFloat) {
                                const float* srcFloat = reinterpret_cast<const float*>(pData);
                                std::copy(srcFloat, srcFloat + (numFramesRead * m_channels), convertedSamples.begin());
                            } else if (pWfx->wBitsPerSample == 16) {
                                const int16_t* src16 = reinterpret_cast<const int16_t*>(pData);
                                for (size_t i = 0; i < numFramesRead * m_channels; ++i) {
                                    convertedSamples[i] = static_cast<float>(src16[i]) / 32768.0f;
                                }
                            } else if (pWfx->wBitsPerSample == 32) {
                                const int32_t* src32 = reinterpret_cast<const int32_t*>(pData);
                                for (size_t i = 0; i < numFramesRead * m_channels; ++i) {
                                    convertedSamples[i] = static_cast<float>(src32[i]) / 2147483648.0f;
                                }
                            } else if (pWfx->wBitsPerSample == 24) {
                                const uint8_t* src24 = reinterpret_cast<const uint8_t*>(pData);
                                for (size_t i = 0; i < numFramesRead * m_channels; ++i) {
                                    int32_t val = (src24[3 * i] << 8) | (src24[3 * i + 1] << 16) | (src24[3 * i + 2] << 24);
                                    convertedSamples[i] = static_cast<float>(val >> 8) / 8388608.0f;
                                }
                            } else {
                                std::fill(convertedSamples.begin(), convertedSamples.end(), 0.0f);
                            }
                        }

                        // Calculate Input RMS level for live VU meter
                        float sumSq = 0.0f;
                        for (float s : convertedSamples) {
                            sumSq += s * s;
                        }
                        float rms = std::sqrt(sumSq / std::max(1.0f, static_cast<float>(convertedSamples.size())));
                        if (m_telemetry) {
                            m_telemetry->PublishInputLevel(std::min(1.0f, rms * 2.5f));
                        }

                        if (m_callback) {
                            m_callback(convertedSamples.data(), numFramesRead, m_channels, m_sampleRate);
                        }
                    }

                    pCaptureClient->ReleaseBuffer(numFramesRead);
                    hr = pCaptureClient->GetNextPacketSize(&packetLength);
                }
            } else {
                // Standby sleep if device is reconnecting
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }

        if (pAudioClient) {
            pAudioClient->Stop();
            pAudioClient->Release();
        }
        if (pCaptureClient) pCaptureClient->Release();
        if (pDevice) pDevice->Release();
        if (pEnumerator) pEnumerator->Release();
        if (pWfx) CoTaskMemFree(pWfx);
        if (hCaptureEvent) CloseHandle(hCaptureEvent);

        if (hTask) AvRevertMmThreadCharacteristics(hTask);
        CoUninitialize();
        logger->Log(Core::LogLevel::Info, "[CaptureEngine] Thread exited.");
    }

}
