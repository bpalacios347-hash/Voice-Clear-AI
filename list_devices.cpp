#include <windows.h>
#include <mmdeviceapi.h>
#include <Functiondiscoverykeys_devpkey.h>
#include <iostream>

int main() {
    CoInitialize(nullptr);
    IMMDeviceEnumerator* pEnum = nullptr;
    CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnum);
    
    IMMDeviceCollection* pCollection = nullptr;
    pEnum->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE, &pCollection);
    
    UINT count = 0;
    pCollection->GetCount(&count);
    
    std::cout << "Capture Devices: " << count << std::endl;
    for (UINT i = 0; i < count; i++) {
        IMMDevice* pDevice = nullptr;
        pCollection->Item(i, &pDevice);
        
        IPropertyStore* pProps = nullptr;
        pDevice->OpenPropertyStore(STGM_READ, &pProps);
        
        PROPVARIANT varName;
        PropVariantInit(&varName);
        pProps->GetValue(PKEY_Device_FriendlyName, &varName);
        
        std::wcout << L" - " << varName.pwszVal << std::endl;
        
        PropVariantClear(&varName);
        pProps->Release();
        pDevice->Release();
    }
    
    pCollection->Release();
    pEnum->Release();
    CoUninitialize();
    return 0;
}
