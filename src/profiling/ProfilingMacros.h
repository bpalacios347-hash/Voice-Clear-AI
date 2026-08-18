#pragma once

#ifdef ENABLE_PROFILING
    #include <tracy/Tracy.hpp>
    
    #define VC_PROFILE_ZONE(name) ZoneScopedN(name)
    #define VC_PROFILE_FRAME(name) FrameMarkNamed(name)
    #define VC_PROFILE_PLOT(name, val) TracyPlot(name, val)
    #define VC_PROFILE_MESSAGE(msg) TracyMessage(msg, strlen(msg))
#else
    #define VC_PROFILE_ZONE(name)
    #define VC_PROFILE_FRAME(name)
    #define VC_PROFILE_PLOT(name, val)
    #define VC_PROFILE_MESSAGE(msg)
#endif
