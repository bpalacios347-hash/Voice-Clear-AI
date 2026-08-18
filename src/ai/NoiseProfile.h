#pragma once
#include <string>

namespace VoiceClear::AI {

    /**
     * @brief Defines studio-grade Noise Profile parameters for Adaptive Spectral DSP & AI.
     */
    struct NoiseProfile {
        std::string name;
        std::string modelOverride;
        float suppressionDepthDb;  // Floor attenuation in dB (-18 to -48 dB)
        float oversubtraction;     // Aggressiveness factor (1.0 to 3.0)
        float voiceBoostDb;        // Vocal presence boost (+0.0 to +3.5 dB)
        int   processingQuality;   // 1=Low, 2=Balanced, 3=High
        bool  bypass;              // True if audio should pass through dry

        // Built-in Studio Profiles
        static NoiseProfile Balanced() {
            // Balanced: -32 dB noise reduction, natural timbre, zero comb filter
            return {"Balanced", "", -32.0f, 1.85f, 0.0f, 2, false};
        }

        static NoiseProfile Strong() {
            // Strong: -48 dB deep reduction, aggressive noise annihilation for loud fans / clicks
            return {"Strong", "", -48.0f, 2.60f, 0.0f, 3, false};
        }

        static NoiseProfile VoicePreservation() {
            // Voice Focus: -24 dB reduction, +2.5 dB vocal presence boost in 1.2 - 3.8 kHz band
            return {"Voice Preservation", "", -24.0f, 1.45f, 2.5f, 2, false};
        }

        static NoiseProfile LowCpu() {
            // Low CPU: -28 dB fast spectral attenuation
            return {"Low CPU", "", -28.0f, 1.65f, 0.0f, 1, false};
        }

        static NoiseProfile Bypass() {
            // Bypass: Completely unaltered passthrough
            return {"Bypass", "", 0.0f, 0.0f, 0.0f, 0, true};
        }
    };

}
