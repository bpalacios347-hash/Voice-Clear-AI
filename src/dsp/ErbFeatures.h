#pragma once
#include <vector>
#include <complex>

namespace VoiceClear {
namespace DSP {

    /**
     * @brief Real-time DSP profile settings for adaptive noise suppression.
     */
    struct DspProfileSettings {
        float suppressionDepthDb{-24.0f}; // Floor attenuation in dB (-18 to -45 dB)
        float oversubtraction{1.35f};      // Spectral over-subtraction factor (1.0 to 2.5)
        float voiceBoostDb{0.0f};         // Vocal presence boost in dB (+0.0 to +3.0 dB)
        bool  bypass{false};              // True if audio should pass through unprocessed
    };

    /**
     * @brief High-performance Adaptive Spectral Wiener & ERB Neural Mask Filter.
     */
    class ErbFeatures {
    public:
        ErbFeatures(int sampleRate, int fftSize, int numErbBands, int numDfBins);
        
        // Calculates ERB features from the complex spectrum (32 bands)
        std::vector<float> ComputeErbFeatures(const std::vector<std::complex<float>>& spectrum);

        // Formats the complex spectrum for feat_spec (2 * 96 = 192 floats)
        std::vector<float> ComputeSpecFeatures(const std::vector<std::complex<float>>& spectrum);

        // Applies Adaptive Spectral Wiener Filter fused with Neural ERB Mask
        std::vector<std::complex<float>> ApplyFilters(
            const std::vector<float>& erbMask, 
            const std::vector<float>& dfCoefs,
            const std::vector<std::complex<float>>& currentSpectrum,
            const DspProfileSettings& settings);

        void ResetState();

    private:
        void InitErbFilterbank();
        void InterpolateErbMask(const std::vector<float>& erbMask, std::vector<float>& binMask);

        int m_sampleRate;
        int m_fftSize;
        int m_numBins;
        int m_numErbBands;
        int m_numDfBins;

        // Non-overlapping Rectangular ERB Bands
        std::vector<int> m_erbBandSizes;

        // Adaptive Noise Estimator & Decision-Directed SNR State
        std::vector<float> m_noisePower;
        std::vector<float> m_priorSpeechPower;
        std::vector<float> m_smoothedGain;
        bool m_initializedNoise{false};
        int  m_frameCount{0};

        // DeepFilterNet Normalization State
        std::vector<float> m_erbNormState;
        std::vector<float> m_specNormState;

        // Deep Filter Complex Delay Line
        // 5 frames history for order-5 complex FIR filtering
        std::vector<std::vector<std::complex<float>>> m_specHistory;
        int m_historyIdx{0};

        // Zero-latency transient energy tracker
        float m_prevEnergy{1e-4f};
    };

}
}
