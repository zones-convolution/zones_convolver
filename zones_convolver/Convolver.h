#pragma once

#include "time_distributed/TimeDistributedNUPC.h"

namespace zones
{
class Convolver
{
public:
    struct ConvolverSpec
    {
        std::vector<int> input_routing;
        std::vector<int> output_routing;
    };

    Convolver (juce::dsp::AudioBlock<const float> ir_block,
               const juce::dsp::ProcessSpec & process_spec,
               const ConvolverSpec & convolver_spec);
    void Process (const juce::dsp::ProcessContextReplacing<float> & replacing);
    void Reset ();

private:
    juce::AudioBuffer<float> routing_buffer_;
    std::unique_ptr<TimeDistributedNUPC> time_distributed_nupc_;

    juce::dsp::ProcessSpec process_spec_;
    ConvolverSpec convolver_spec_;
    int num_convolution_channels_;
};
}