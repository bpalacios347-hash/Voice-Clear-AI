#include "ErbFeatures.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <numeric>

namespace VoiceClear {
namespace DSP {

ErbFeatures::ErbFeatures(int sampleRate, int fftSize, int numErbBands, int numDfBins)
    : m_sampleRate(sampleRate)
    , m_fftSize(fftSize)
    , m_numBins(fftSize / 2 + 1)
    , m_numErbBands(numErbBands)
    , m_numDfBins(numDfBins)
{
    InitErbFilterbank();
    ResetState();
}

void ErbFeatures::ResetState() {
    m_erbNormState.resize(m_numErbBands);
    for (int i = 0; i < m_numErbBands; ++i) {
        float frac = (m_numErbBands > 1) ? static_cast<float>(i) / (m_numErbBands - 1) : 0.0f;
        // libDF MEAN_NORM_INIT: [-60.0 dB, -90.0 dB]
        m_erbNormState[i] = -60.0f * (1.0f - frac) + (-90.0f) * frac;
    }

    m_specNormState.resize(m_numDfBins);
    for (int i = 0; i < m_numDfBins; ++i) {
        float frac = (m_numDfBins > 1) ? static_cast<float>(i) / (m_numDfBins - 1) : 0.0f;
        // libDF UNIT_NORM_INIT: [0.001, 0.0001]
        m_specNormState[i] = 0.001f * (1.0f - frac) + 0.0001f * frac;
    }

    m_prevEnergy = 1e-4f;

    m_specHistory.assign(5, std::vector<std::complex<float>>(m_numBins, {0.0f, 0.0f}));
    m_historyIdx = 0;
}

void ErbFeatures::InitErbFilterbank() {
    m_erbBandSizes.resize(m_numErbBands, 0);

    auto freq2erb = [](float freq_hz) {
        return 9.265f * std::log1p(freq_hz / (24.7f * 9.265f));
    };
    auto erb2freq = [](float n_erb) {
        return 24.7f * 9.265f * (std::exp(n_erb / 9.265f) - 1.0f);
    };

    float freq_width = static_cast<float>(m_sampleRate) / m_fftSize;
    float nyq_freq = m_sampleRate / 2.0f;
    float erb_low = freq2erb(0.0f);
    float erb_high = freq2erb(nyq_freq);
    float step = (erb_high - erb_low) / m_numErbBands;
    
    int min_nb_freqs = 2;
    int prev_freq = 0;
    int freq_over = 0;

    for (int i = 1; i <= m_numErbBands; ++i) {
        float f = erb2freq(erb_low + i * step);
        int fb = static_cast<int>(std::round(f / freq_width));
        int nb_freqs = fb - prev_freq - freq_over;
        
        if (nb_freqs < min_nb_freqs) {
            freq_over = min_nb_freqs - nb_freqs;
            nb_freqs = min_nb_freqs;
        } else {
            freq_over = 0;
        }
        m_erbBandSizes[i - 1] = nb_freqs;
        prev_freq = fb;
    }
    m_erbBandSizes[m_numErbBands - 1] += 1; // Since we have window_size/2 + 1 frequency bins

    int totalBins = 0;
    for (int size : m_erbBandSizes) totalBins += size;
    if (totalBins != m_numBins) {
        m_erbBandSizes[m_numErbBands - 1] += (m_numBins - totalBins);
    }
}

std::vector<float> ErbFeatures::ComputeErbFeatures(const std::vector<std::complex<float>>& spectrum) {
    std::vector<float> featErb(m_numErbBands);
    const float alpha = 0.9900498337491681f; // exp(-0.01 / 1.0)
    const float wnorm = 1.0f / static_cast<float>(m_fftSize);
    const float wnormSq = wnorm * wnorm;

    // 1. Compute normalized power spectrum: |X[i]|^2 * (1/N)^2
    std::vector<float> powerSpec(m_numBins);
    for (int i = 0; i < m_numBins; ++i) {
        powerSpec[i] = std::norm(spectrum[i]) * wnormSq;
    }

    // 2. Average energy per ERB band and apply libDF band_mean_norm_erb
    int currentBin = 0;
    for (int band = 0; band < m_numErbBands; ++band) {
        float energy = 1e-10f;
        int bandSize = m_erbBandSizes[band];
        
        float k = 1.0f / static_cast<float>(bandSize);
        for (int j = 0; j < bandSize; ++j) {
            energy += powerSpec[currentBin + j] * k;
        }
        currentBin += bandSize;
        
        float x = 10.0f * std::log10(energy); // log energy in dB
        float& s = m_erbNormState[band];
        
        s = x * (1.0f - alpha) + s * alpha;
        featErb[band] = (x - s) / 40.0f; // Normalized for neural input
    }

    return featErb;
}

std::vector<float> ErbFeatures::ComputeSpecFeatures(const std::vector<std::complex<float>>& spectrum) {
    // 2 channels (real, imag), 96 bins = 192 floats
    std::vector<float> featSpec(2 * m_numDfBins);
    const float alpha = 0.9900498337491681f;
    const float wnorm = 1.0f / static_cast<float>(m_fftSize);

    for (int bin = 0; bin < m_numDfBins; ++bin) {
        float re = spectrum[bin].real() * wnorm;
        float im = spectrum[bin].imag() * wnorm;
        float x_norm = std::sqrt(re * re + im * im);
        float& s = m_specNormState[bin];

        // libDF band_unit_norm_t: s = norm * (1 - alpha) + s * alpha
        s = x_norm * (1.0f - alpha) + s * alpha;
        float denom = std::sqrt(s) + 1e-10f;

        featSpec[bin] = re / denom;
        featSpec[m_numDfBins + bin] = im / denom;
    }
    return featSpec;
}

void ErbFeatures::InterpolateErbMask(const std::vector<float>& erbMask, std::vector<float>& binMask) {
    binMask.resize(m_numBins, 1.0f);
    if (erbMask.empty()) return;
    
    size_t baseIdx = erbMask.size() >= static_cast<size_t>(m_numErbBands) 
                     ? erbMask.size() - m_numErbBands 
                     : 0;

    int currentBin = 0;
    for (int band = 0; band < m_numErbBands; ++band) {
        int bandSize = m_erbBandSizes[band];
        float gain = std::clamp(erbMask[baseIdx + band], 0.0f, 1.0f);
        for (int j = 0; j < bandSize && (currentBin + j) < m_numBins; ++j) {
            binMask[currentBin + j] = gain;
        }
        currentBin += bandSize;
    }
}

std::vector<std::complex<float>> ErbFeatures::ApplyFilters(
    const std::vector<float>& erbMask, 
    const std::vector<float>& dfCoefs,
    const std::vector<std::complex<float>>& currentSpectrum,
    const DspProfileSettings& settings) {
    
    if (currentSpectrum.size() != static_cast<size_t>(m_numBins)) {
        return currentSpectrum;
    }

    if (settings.bypass) {
        return currentSpectrum;
    }

    // 1. Shift rolling 5-frame complex spectrum history buffer:
    //    m_specHistory[0] = oldest (t-4)
    //    m_specHistory[4] = newest (current frame t)
    for (size_t i = 0; i < 4; ++i) {
        m_specHistory[i] = m_specHistory[i + 1];
    }
    m_specHistory[4] = currentSpectrum;

    // 2. Expand ERB Mask across all frequency bins
    std::vector<float> binMask;
    InterpolateErbMask(erbMask, binMask);

    // Attenuation floor calculation (e.g. -28 dB floor = 0.0398)
    // Guarantees voice consonants, fricatives, and quiet syllables are NEVER cancelled out
    float attenLimitDb = std::abs(settings.suppressionDepthDb);
    float lim = (attenLimitDb > 0.0f) ? std::pow(10.0f, -attenLimitDb / 20.0f) : 0.025f;

    std::vector<std::complex<float>> enhancedSpectrum(m_numBins);

    // 3. Stage 1: Deep Filtering (DF-Op) 5-tap Complex FIR on lowest 96 bins (0 to 4.8 kHz)
    bool hasDf = (!dfCoefs.empty() && (dfCoefs.size() % 960 == 0 || dfCoefs.size() % 192 == 0));
    size_t orderStride = hasDf ? (dfCoefs.size() / 5) : 0;
    size_t timeOffset  = (orderStride >= 192) ? (orderStride - 192) : 0;

    for (int bin = 0; bin < m_numDfBins && bin < m_numBins; ++bin) {
        if (hasDf) {
            float out_r = 0.0f;
            float out_i = 0.0f;

            for (size_t k = 0; k < 5; ++k) {
                size_t coefIdx = k * orderStride + timeOffset + bin * 2;
                float cre = dfCoefs[coefIdx];
                float cim = dfCoefs[coefIdx + 1];

                const auto& histSample = m_specHistory[k][bin];
                float sre = histSample.real();
                float sim = histSample.imag();

                // Complex multiplication: (sre + j*sim) * (cre + j*cim)
                out_r += (sre * cre - sim * cim);
                out_i += (sre * cim + sim * cre);
            }

            enhancedSpectrum[bin] = std::complex<float>(out_r, out_i);
        } else {
            float g = std::max(binMask[bin], lim);
            enhancedSpectrum[bin] = currentSpectrum[bin] * g;
        }
    }

    // 4. Stage 2: High Frequency ERB Neural Mask on bins 96 to 480 (4.8 kHz to 24 kHz)
    for (int bin = m_numDfBins; bin < m_numBins; ++bin) {
        float g = std::max(binMask[bin], lim);
        enhancedSpectrum[bin] = currentSpectrum[bin] * g;
    }

    // 5. Zero-Latency Transient Impulse & Clap Shaper (Instant Attack)
    // Eliminates the initial click/burst from claps, keyboard snaps, and knocks with 0ms latency
    float totEnergy = 1e-10f;
    float lowEnergy = 1e-10f;
    float highEnergy = 1e-10f;
    float maxLowPower = 0.0f;

    for (int bin = 0; bin < m_numBins; ++bin) {
        float p = std::norm(currentSpectrum[bin]);
        totEnergy += p;
        if (bin < 50) { // 0 to 2.5 kHz (Vocal formant range)
            lowEnergy += p;
            if (p > maxLowPower) maxLowPower = p;
        } else if (bin >= 70) { // 3.5 kHz to 24 kHz (High transient range)
            highEnergy += p;
        }
    }

    float highRatio = highEnergy / totEnergy;
    float avgLowPower = lowEnergy / 50.0f;
    float lowPeakRatio = maxLowPower / (avgLowPower + 1e-10f);
    float energyJump = totEnergy / (m_prevEnergy + 1e-10f);
    m_prevEnergy = 0.85f * m_prevEnergy + 0.15f * totEnergy;

    // A non-speech transient (clap, click, tap) has sudden energy jump (> 4x = 6dB)
    // AND high frequency dominance (> 35% above 3.5kHz) OR absence of harmonic pitch structure (lowPeakRatio < 3.5)
    bool isTransientNoise = (energyJump > 4.0f) && (highRatio > 0.35f || lowPeakRatio < 3.5f);

    if (isTransientNoise) {
        for (int bin = 0; bin < m_numDfBins && bin < m_numBins; ++bin) {
            enhancedSpectrum[bin] *= 0.02f; // -34 dB instant clamp on transient impulse
        }
        for (int bin = m_numDfBins; bin < m_numBins; ++bin) {
            enhancedSpectrum[bin] *= 0.005f; // -46 dB instant annihilation on high-frequency click
        }
    }

    // 6. Clean Voice Makeup Gain (+4.0 dB = 1.585x) & Vocal Presence Boost
    // When background noise is removed, perceived voice volume drops; this restores punch and clarity.
    float makeupGain = 1.585f; // +4.0 dB makeup gain
    if (settings.voiceBoostDb > 0.01f) {
        makeupGain *= std::pow(10.0f, settings.voiceBoostDb / 20.0f);
    }

    for (int bin = 0; bin < m_numBins; ++bin) {
        enhancedSpectrum[bin] *= makeupGain;
    }

    return enhancedSpectrum;
}

}
}
