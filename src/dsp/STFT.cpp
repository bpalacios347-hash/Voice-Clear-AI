#include "STFT.h"
#include <kissfft/kiss_fftr.h>
#include <cmath>
#include <stdexcept>
#include <algorithm>

namespace VoiceClear {
namespace DSP {

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

STFT::STFT(int frameSize, int fftSize)
    : m_frameSize(frameSize), m_fftSize(fftSize) {
    if (fftSize % 2 != 0 || fftSize < frameSize) {
        throw std::invalid_argument("FFT size must be even and >= frameSize");
    }

    int numBins = m_fftSize / 2 + 1;

    m_window.resize(fftSize, 0.0f);
    m_overlapBufferIn.resize(fftSize - frameSize, 0.0f);
    m_overlapBufferOut.resize(fftSize - frameSize, 0.0f);

    // Pre-allocate zero-allocation scratch buffers
    m_fftInput.resize(fftSize, 0.0f);
    m_spectrum.resize(numBins, {0.0f, 0.0f});
    m_timeOut.resize(fftSize, 0.0f);
    m_outputFrame.resize(frameSize, 0.0f);

    ComputeHannWindow();

    m_kissFftCfg = kiss_fftr_alloc(fftSize, 0, nullptr, nullptr);
    m_kissIfftCfg = kiss_fftr_alloc(fftSize, 1, nullptr, nullptr);
}

STFT::~STFT() {
    kiss_fftr_free((kiss_fftr_cfg)m_kissFftCfg);
    kiss_fftr_free((kiss_fftr_cfg)m_kissIfftCfg);
}

void STFT::ComputeHannWindow() {
    int window_size_h = m_fftSize / 2;
    for (int i = 0; i < m_fftSize; ++i) {
        // Exact Vorbis window from DeepFilterNet / libDF: sin(pi/2 * sin^2(pi * (i + 0.5) / N))
        double sinVal = std::sin(0.5 * M_PI * (i + 0.5) / window_size_h);
        m_window[i] = static_cast<float>(std::sin(0.5 * M_PI * sinVal * sinVal));
    }
}

const std::vector<std::complex<float>>& STFT::Forward(std::span<const float> inputFrame) {
    int overlapSize = m_fftSize - m_frameSize;

    // Shift in new data: [old_data..., new_data...]
    std::copy(m_overlapBufferIn.begin(), m_overlapBufferIn.end(), m_fftInput.begin());
    std::copy(inputFrame.begin(), inputFrame.end(), m_fftInput.begin() + overlapSize);

    // Save for next hop
    std::copy(m_fftInput.begin() + m_frameSize, m_fftInput.end(), m_overlapBufferIn.begin());

    // Apply Analysis Window
    for (int i = 0; i < m_fftSize; ++i) {
        m_fftInput[i] *= m_window[i];
    }

    kiss_fftr((kiss_fftr_cfg)m_kissFftCfg, m_fftInput.data(), reinterpret_cast<kiss_fft_cpx*>(m_spectrum.data()));

    return m_spectrum;
}

const std::vector<float>& STFT::Inverse(std::span<const std::complex<float>> spectrum) {
    kiss_fftri((kiss_fftr_cfg)m_kissIfftCfg, 
               reinterpret_cast<const kiss_fft_cpx*>(spectrum.data()), 
               m_timeOut.data());

    int overlapSize = m_fftSize - m_frameSize;
    float normFactor = 1.0f / m_fftSize;

    // Apply Synthesis Window, Normalize, and Overlap-Add
    for (int i = 0; i < m_frameSize; ++i) {
        float currentVal = m_timeOut[i] * m_window[i] * normFactor;
        m_outputFrame[i] = m_overlapBufferOut[i] + currentVal;
    }

    // Save the tail for the next overlap
    for (int i = 0; i < overlapSize; ++i) {
        m_overlapBufferOut[i] = m_timeOut[i + m_frameSize] * m_window[i + m_frameSize] * normFactor;
    }

    return m_outputFrame;
}

}
}
