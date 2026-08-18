#pragma once

#include <ntddk.h>
#include <windef.h>
#include <ks.h>
#include <ksmedia.h>

#define POOLTAG_VOICECLEAR 'rlCV'

// For kernel mode float usage
extern "C" {
    __declspec(selectany) int _fltused = 0;
}

// External references for AVStream descriptors
extern const KSDEVICE_DESCRIPTOR DeviceDescriptor;
extern const KSFILTER_DESCRIPTOR FilterDescriptor;

// Global driver functions
extern "C" DRIVER_INITIALIZE DriverEntry;

// PnP and Power callbacks removed (using AVStream defaults)

inline void* __cdecl operator new(size_t size, POOL_TYPE poolType, ULONG tag) { return ExAllocatePool2(POOL_FLAG_NON_PAGED, size, tag); }
inline void __cdecl operator delete(void* p) { if (p) ExFreePool(p); }
inline void __cdecl operator delete(void* p, size_t) { if (p) ExFreePool(p); }
