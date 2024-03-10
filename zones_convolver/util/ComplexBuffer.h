#pragma once
#include <juce_dsp/juce_dsp.h>

namespace zones
{
class ComplexBuffer
{
public:
    ComplexBuffer (std::size_t num_points, std::size_t num_channels);
    [[nodiscard]] std::size_t GetNumChannels () const;
    [[nodiscard]] std::size_t GetNumPoints () const;

    [[nodiscard]] const std::complex<float> * GetReadPointer (std::size_t channel_index) const;
    std::complex<float> * GetWritePointer (std::size_t channel_index);

    [[nodiscard]] const std::complex<float> * const * GetArrayOfReadPointer () const;
    std::complex<float> * const * GetArrayOfWritePointer ();

    [[nodiscard]] juce::dsp::AudioBlock<float> GetContinuousBlock ();
    [[nodiscard]] juce::dsp::AudioBlock<const float> GetContinuousBlock () const;

    void Clear ();
    void ComplexMultiplyFrom (const ComplexBuffer & a, const ComplexBuffer & b);
    void AddFrom (const ComplexBuffer & buffer);
    void ComplexMultiplyAccumulateFrom (const ComplexBuffer & a, const ComplexBuffer & b);
    void CopyFromAudioBlock (juce::dsp::AudioBlock<const float> block);

private:
    std::size_t num_channels_ = 0;
    std::size_t num_points_ = 0;
    std::vector<std::complex<float>> channel_data_;
    std::vector<std::complex<float> *> channel_pointers_;
};
}