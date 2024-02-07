#pragma once

#include <juce_dsp/juce_dsp.h>

class TimeDomainConvolver : public juce::dsp::ProcessorBase
{
public:
    void prepare (const juce::dsp::ProcessSpec & spec) override;
    void process (const juce::dsp::ProcessContextReplacing<float> & replacing) override;
    void reset () override;
    void LoadImpulseResponse (juce::dsp::AudioBlock<float> ir_block, double sample_rate);

private:
    juce::AudioBuffer<float> ir_buffer_;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::None> delay_line_;
};
