#include "Driver.h"

const KSNODE_DESCRIPTOR NodeDescriptors[] = {
    {
        nullptr,         // Automation table
        &KSNODETYPE_ADC, // Type
        nullptr          // Name
    }
};

// Define the topology connections
const KSTOPOLOGY_CONNECTION FilterConnections[] = {
    { KSFILTER_NODE, 1, 0, 1 }, // Pin 1 (Microphone IN) -> Node 0 (ADC) Pin 1 (IN)
    { 0, 0, KSFILTER_NODE, 0 }  // Node 0 (ADC) Pin 0 (OUT) -> Pin 0 (Capture OUT)
};

const KSFILTER_DISPATCH FilterDispatch = {
    nullptr, // Create
    nullptr, // Close
    nullptr, // Process
    nullptr  // Reset
};

// Forward declaration of the Pin descriptors defined in Pin.cpp
extern const KSPIN_DESCRIPTOR_EX PinDescriptors[];
extern const ULONG PinDescriptorCount;

const GUID FilterCategories[] = {
    {0x6994AD04L, 0x93EF, 0x11D0, {0xA3, 0xCC, 0x00, 0xA0, 0xC9, 0x22, 0x31, 0x96}}, // KSCATEGORY_AUDIO
    {0x65E8773DL, 0x8F56, 0x11D0, {0xA3, 0xB9, 0x00, 0xA0, 0xC9, 0x22, 0x31, 0x96}}  // KSCATEGORY_CAPTURE
};

// Unique Reference GUID to bypass Endpoint Builder cache
const GUID KSNAME_VoiceClearFilter = {0xff70ebf1, 0x7a01, 0x4166, {0x94, 0xe4, 0x96, 0xba, 0x40, 0x7d, 0x9d, 0x36}};

const KSFILTER_DESCRIPTOR FilterDescriptor = {
    &FilterDispatch,
    nullptr, // AutomationTable
    KSFILTER_DESCRIPTOR_VERSION,
    0, // Flags
    &KSNAME_VoiceClearFilter, // ReferenceGuid
    PinDescriptorCount, // PinDescriptorsCount
    sizeof(KSPIN_DESCRIPTOR_EX), // PinDescriptorSize
    PinDescriptors, // PinDescriptors
    ARRAYSIZE(FilterCategories), // CategoriesCount
    FilterCategories, // Categories
    ARRAYSIZE(NodeDescriptors), // NodeDescriptorsCount
    sizeof(KSNODE_DESCRIPTOR), // NodeDescriptorSize
    NodeDescriptors, // NodeDescriptors
    ARRAYSIZE(FilterConnections), // ConnectionsCount
    FilterConnections, // Connections
    nullptr // ComponentId
};

// Register KSDEVICE descriptors
extern const KSDEVICE_DESCRIPTOR DeviceDescriptor;

NTSTATUS IntersectHandler(
    PKSFILTER Filter,
    PIRP Irp,
    PKSP_PIN PinInstance,
    PKSDATARANGE CallerDataRange,
    PKSDATARANGE DescriptorDataRange,
    ULONG BufferSize,
    PVOID Data OPTIONAL,
    PULONG DataSize)
{
    // Return standard Float32 48kHz format intersection
    return STATUS_SUCCESS;
}
