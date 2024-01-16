#pragma once
#include <juce_dsp/juce_dsp.h>

class SplitBlock
{
public:
    SplitBlock (juce::dsp::AudioBlock<float> first_block,
                juce::dsp::AudioBlock<float> wrapped_block);

    void CopyFrom (juce::dsp::AudioBlock<const float> block);
    void AddFrom (juce::dsp::AudioBlock<const float> block);

    void CopyTo (juce::dsp::AudioBlock<float> block);
    void Clear ();

    SplitBlock GetSubBlock (std::size_t num_samples);

private:
    juce::dsp::AudioBlock<float> first_block_;
    juce::dsp::AudioBlock<float> wrapped_block_;

    std::size_t total_num_samples_ = 0;
};

class CircularBuffer
{
public:
    explicit CircularBuffer (juce::AudioBuffer<float> & buffer);
    SplitBlock GetNext (std::size_t advancement);

private:
    std::size_t head_position_ = 0;
    juce::AudioBuffer<float> & buffer_;
};

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
    void ComplexMultiplyAccumulateFrom (const ComplexBuffer & a, const ComplexBuffer & b);
    void CopyFromAudioBlock (juce::dsp::AudioBlock<const float> block);

private:
    std::size_t num_channels_ = 0;
    std::size_t num_points_ = 0;
    std::vector<std::complex<float>> channel_data_;
    std::vector<std::complex<float> *> channel_pointers_;
};

class FrequencyDelayLine
{
public:
    FrequencyDelayLine (std::size_t num_channels,
                        std::size_t num_blocks,
                        std::size_t num_points_per_block);

    ComplexBuffer & GetNextBlock ();
    [[nodiscard]] const ComplexBuffer & GetBlockWithOffset (std::size_t offset) const;

private:
    std::vector<ComplexBuffer> delay_line_;
    std::size_t head_position_ = 0;
    std::size_t num_blocks_;
};