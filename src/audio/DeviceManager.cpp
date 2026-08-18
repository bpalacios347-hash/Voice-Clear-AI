#include "DeviceManager.h"
#include "../utils/Logger.h"
#include <windows.h>
#include <mmdeviceapi.h>
#include <Functiondiscoverykeys_devpkey.h>
#include <comdef.h>

namespace VoiceClear::Audio {

    class DeviceManager::NotificationClient : public IMMNotificationClient {
    public:
        NotificationClient(Diagnostics::LockFreeTelemetryQueue<DeviceEvent>& queue) 
            : m_refCount(1), m_queue(queue) {}
        ~NotificationClient() = default;

        ULONG STDMETHODCALLTYPE AddRef() override {
            return InterlockedIncrement(&m_refCount);
        }

        ULONG STDMETHODCALLTYPE Release() override {
            ULONG ulRef = InterlockedDecrement(&m_refCount);
            if (0 == ulRef) {
                delete this;
            }
            return ulRef;
        }

        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, VOID **ppvInterface) override {
            if (IID_IUnknown == riid) {
                AddRef();
                *ppvInterface = (IUnknown*)this;
            } else if (__uuidof(IMMNotificationClient) == riid) {
                AddRef();
                *ppvInterface = (IMMNotificationClient*)this;
            } else {
                *ppvInterface = NULL;
                return E_NOINTERFACE;
            }
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState) override {
            char id[512];
            WideCharToMultiByte(CP_UTF8, 0, pwstrDeviceId, -1, id, sizeof(id), NULL, NULL);
            m_queue.Push({DeviceEventType::StateChanged, std::string(id), DataFlow::All});
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR pwstrDeviceId) override {
            char id[512];
            WideCharToMultiByte(CP_UTF8, 0, pwstrDeviceId, -1, id, sizeof(id), NULL, NULL);
            m_queue.Push({DeviceEventType::Added, std::string(id), DataFlow::All});
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR pwstrDeviceId) override {
            char id[512];
            WideCharToMultiByte(CP_UTF8, 0, pwstrDeviceId, -1, id, sizeof(id), NULL, NULL);
            m_queue.Push({DeviceEventType::Removed, std::string(id), DataFlow::All});
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDefaultDeviceId) override {
            if (pwstrDefaultDeviceId) {
                char id[512];
                WideCharToMultiByte(CP_UTF8, 0, pwstrDefaultDeviceId, -1, id, sizeof(id), NULL, NULL);
                m_queue.Push({DeviceEventType::DefaultChanged, std::string(id), (flow == eCapture) ? DataFlow::Capture : DataFlow::Render});
            }
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key) override {
            return S_OK;
        }

    private:
        LONG m_refCount;
        Diagnostics::LockFreeTelemetryQueue<DeviceEvent>& m_queue;
    };

    DeviceManager::DeviceManager() 
        : m_eventQueue(1024) {}

    DeviceManager::~DeviceManager() {
        Shutdown();
    }

    bool DeviceManager::Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }

    bool DeviceManager::Start() {
        if (!m_initialized || m_running) return false;
        
        m_running = true;
        
        // Start COM thread
        m_comThread = std::thread(&DeviceManager::ComThreadLoop, this);
        
        // Start worker thread for event dispatching
        m_workerThread = std::thread(&DeviceManager::WorkerThreadLoop, this);

        return true;
    }

    void DeviceManager::Stop() {
        if (!m_running) return;
        m_running = false;

        if (m_comThread.joinable()) m_comThread.join();
        if (m_workerThread.joinable()) m_workerThread.join();
    }

    void DeviceManager::Shutdown() {
        Stop();
        m_initialized = false;
    }

    void DeviceManager::RegisterEventCallback(DeviceChangeCallback callback) {
        m_callback = callback;
    }

    std::vector<AudioEndpointInfo> DeviceManager::EnumerateInputDevices() {
        std::vector<AudioEndpointInfo> devices;
        
        // To be real-time safe and strictly decoupled, enumeration should ideally 
        // happen on the COM thread or assume caller is a safe configuration thread.
        // For production, we temporarily init COM here if not on COM thread, 
        // as enumeration is usually done once on startup by the UI/Config thread.
        HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
        bool coInitLocal = SUCCEEDED(hr);

        IMMDeviceEnumerator* pEnumerator = NULL;
        hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
        if (FAILED(hr)) {
            if (coInitLocal) CoUninitialize();
            return devices;
        }

        IMMDeviceCollection* pCollection = NULL;
        hr = pEnumerator->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE | DEVICE_STATE_DISABLED | DEVICE_STATE_UNPLUGGED, &pCollection);
        if (SUCCEEDED(hr)) {
            UINT count = 0;
            pCollection->GetCount(&count);
            for (UINT i = 0; i < count; i++) {
                IMMDevice* pDevice = NULL;
                if (SUCCEEDED(pCollection->Item(i, &pDevice))) {
                    LPWSTR pwszID = NULL;
                    pDevice->GetId(&pwszID);
                    
                    IPropertyStore* pProps = NULL;
                    std::string friendlyName = "Unknown";
                    if (SUCCEEDED(pDevice->OpenPropertyStore(STGM_READ, &pProps))) {
                        PROPVARIANT varName;
                        PropVariantInit(&varName);
                        if (SUCCEEDED(pProps->GetValue(PKEY_Device_FriendlyName, &varName))) {
                            char name[512];
                            WideCharToMultiByte(CP_UTF8, 0, varName.pwszVal, -1, name, sizeof(name), NULL, NULL);
                            friendlyName = name;
                        }
                        PropVariantClear(&varName);
                        pProps->Release();
                    }

                    DWORD state;
                    pDevice->GetState(&state);
                    DeviceState ds = DeviceState::Active;
                    if (state & DEVICE_STATE_DISABLED) ds = DeviceState::Disabled;
                    else if (state & DEVICE_STATE_UNPLUGGED) ds = DeviceState::Unplugged;
                    else if (state & DEVICE_STATE_NOTPRESENT) ds = DeviceState::NotPresent;

                    char id[512];
                    WideCharToMultiByte(CP_UTF8, 0, pwszID, -1, id, sizeof(id), NULL, NULL);
                    
                    devices.push_back({std::string(id), friendlyName, ds, DataFlow::Capture, false, false});
                    
                    CoTaskMemFree(pwszID);
                    pDevice->Release();
                }
            }
            pCollection->Release();
        }

        pEnumerator->Release();
        if (coInitLocal) {
            CoUninitialize();
        }
        
        return devices;
    }

    std::vector<AudioEndpointInfo> DeviceManager::EnumerateOutputDevices() {
        std::vector<AudioEndpointInfo> devices;
        
        HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
        bool coInitLocal = SUCCEEDED(hr);

        IMMDeviceEnumerator* pEnumerator = NULL;
        hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
        if (FAILED(hr)) {
            if (coInitLocal) CoUninitialize();
            return devices;
        }

        IMMDeviceCollection* pCollection = NULL;
        hr = pEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE | DEVICE_STATE_DISABLED | DEVICE_STATE_UNPLUGGED, &pCollection);
        if (SUCCEEDED(hr)) {
            UINT count = 0;
            pCollection->GetCount(&count);
            for (UINT i = 0; i < count; i++) {
                IMMDevice* pDevice = NULL;
                if (SUCCEEDED(pCollection->Item(i, &pDevice))) {
                    LPWSTR pwszID = NULL;
                    pDevice->GetId(&pwszID);
                    
                    IPropertyStore* pProps = NULL;
                    std::string friendlyName = "Unknown";
                    if (SUCCEEDED(pDevice->OpenPropertyStore(STGM_READ, &pProps))) {
                        PROPVARIANT varName;
                        PropVariantInit(&varName);
                        if (SUCCEEDED(pProps->GetValue(PKEY_Device_FriendlyName, &varName))) {
                            char name[512];
                            WideCharToMultiByte(CP_UTF8, 0, varName.pwszVal, -1, name, sizeof(name), NULL, NULL);
                            friendlyName = name;
                        }
                        PropVariantClear(&varName);
                        pProps->Release();
                    }

                    DWORD state;
                    pDevice->GetState(&state);
                    DeviceState ds = DeviceState::Active;
                    if (state & DEVICE_STATE_DISABLED) ds = DeviceState::Disabled;
                    else if (state & DEVICE_STATE_UNPLUGGED) ds = DeviceState::Unplugged;
                    else if (state & DEVICE_STATE_NOTPRESENT) ds = DeviceState::NotPresent;

                    char id[512];
                    WideCharToMultiByte(CP_UTF8, 0, pwszID, -1, id, sizeof(id), NULL, NULL);
                    
                    devices.push_back({std::string(id), friendlyName, ds, DataFlow::Render, false, false});
                    
                    CoTaskMemFree(pwszID);
                    pDevice->Release();
                }
            }
            pCollection->Release();
        }

        pEnumerator->Release();
        if (coInitLocal) {
            CoUninitialize();
        }
        
        return devices;
    }

    void DeviceManager::ComThreadLoop() {
        auto logger = Utils::Logger::GetInstance();
        logger->Log(Core::LogLevel::Debug, "DeviceManager COM Thread started.");

        HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
        if (FAILED(hr)) {
            logger->Log(Core::LogLevel::Error, "Failed to initialize COM on DeviceManager thread.");
            return;
        }

        IMMDeviceEnumerator* pEnumerator = NULL;
        hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
        if (SUCCEEDED(hr)) {
            m_notificationClient = std::make_unique<NotificationClient>(m_eventQueue);
            pEnumerator->RegisterEndpointNotificationCallback(m_notificationClient.get());

            // Simple event loop pumping
            while (m_running) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }

            pEnumerator->UnregisterEndpointNotificationCallback(m_notificationClient.get());
            pEnumerator->Release();
            m_notificationClient.reset();
        }

        CoUninitialize();
        logger->Log(Core::LogLevel::Debug, "DeviceManager COM Thread exiting.");
    }

    void DeviceManager::WorkerThreadLoop() {
        auto logger = Utils::Logger::GetInstance();
        logger->Log(Core::LogLevel::Debug, "DeviceManager Worker Thread started.");

        while (m_running) {
            DeviceEvent ev;
            while (m_eventQueue.Pop(ev)) {
                if (m_callback) {
                    m_callback(ev);
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        logger->Log(Core::LogLevel::Debug, "DeviceManager Worker Thread exiting.");
    }

}
