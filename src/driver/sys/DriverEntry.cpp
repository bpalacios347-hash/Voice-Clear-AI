#include "Driver.h"

#pragma code_seg("INIT")

extern "C" NTSTATUS DriverEntry(
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegistryPath
) {
    NTSTATUS status;

    // Register with AVStream
    status = KsInitializeDriver(
        DriverObject,
        RegistryPath,
        &DeviceDescriptor
    );

    if (!NT_SUCCESS(status)) {
        return status;
    }

    return STATUS_SUCCESS;
}

#pragma code_seg()

// Device descriptor implementation
NTSTATUS StartDevice(PKSDEVICE Device, PIRP Irp, PCM_RESOURCE_LIST TranslatedResourceList, PCM_RESOURCE_LIST UntranslatedResourceList);

const KSDEVICE_DISPATCH DeviceDispatch = {
    nullptr, // AddDevice
    StartDevice,
    nullptr, // PostStart
    nullptr, // QueryStop
    nullptr, // CancelStop
    nullptr, // StopDevice
    nullptr, // QueryRemove
    nullptr, // CancelRemove
    nullptr, // Remove
    nullptr, // QueryCapabilities
    nullptr, // SurpriseRemoval
    nullptr, // QueryPower
    nullptr  // SetPower
};

const KSFILTER_DESCRIPTOR* const DeviceFilterDescriptors[] = {
    &FilterDescriptor
};

const KSDEVICE_DESCRIPTOR DeviceDescriptor = {
    &DeviceDispatch,
    ARRAYSIZE(DeviceFilterDescriptors),
    DeviceFilterDescriptors
};


NTSTATUS StartDevice(PKSDEVICE Device, PIRP Irp, PCM_RESOURCE_LIST TranslatedResourceList, PCM_RESOURCE_LIST UntranslatedResourceList) { UNREFERENCED_PARAMETER(Irp); UNREFERENCED_PARAMETER(TranslatedResourceList); UNREFERENCED_PARAMETER(UntranslatedResourceList); PKSFILTERFACTORY FilterFactory = KsDeviceGetFirstChildFilterFactory(Device); if (FilterFactory) { KsFilterFactorySetDeviceClassesState(FilterFactory, TRUE); } return STATUS_SUCCESS; }
