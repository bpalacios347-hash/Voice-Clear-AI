#pragma once
#include <vector>
#include <complex>
#include <span>

namespace VoiceClear {
namespace DSP {

class STFT {
public:
    STFT(int frameSize, int fftSize);
    ~STFT();

    // Takes a frame of size `frameSize` and performs windowing, zero-padding (if needed) 
    // and returns the complex spectrum (fftSize/2 + 1 bins)
    const std::vector<std::complex<float>>& Forward(std::span<const float> inputFrame);

    // Takes the complex spectrum (fftSize/2 + 1 bins), performs iFFT, windowing,
    // overlap-add, and returns the next output frame of size `frameSize`.
    const std::vector<float>& Inverse(std::span<const std::complex<float>> spectrum);

    int GetFftSize() const { return m_fftSize; }
    int GetFrameSize() const { return m_frameSize; }

private:
    void ComputeHannWindow();

    int m_frameSize;
    int m_fftSize;

    std::vector<float> m_window;
    std::vector<float> m_overlapBufferIn;
    std::vector<float> m_overlapBufferOut;

    // Zero-allocation scratch buffers
    std::vector<float> m_fftInput;
    std::vector<std::complex<float>> m_spectrum;
    std::vector<float> m_timeOut;
    std::vector<float> m_outputFrame;
    
    // KissFFT states
    void* m_kissFftCfg;
    void* m_kissIfftCfg;
};

}
}
