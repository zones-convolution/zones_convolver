#pragma once

class TimeDistributedNUPC
{
public:
    TimeDistributedNUPC (juce::dsp::AudioBlock<const float> ir_buffer,
                         const juce::dsp::ProcessSpec & process_spec);
    void Process (const juce::dsp::ProcessContextReplacing<float> & replacing);
    void Reset ();

private:
};
