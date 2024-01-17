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