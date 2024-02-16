#pragma once

#include "TimeDistributedUPC.h"
#include "UniformPartitionedConvolver.h"
#include "util/CircularBuffer.h"

#include <juce_dsp/juce_dsp.h>

class NonUniformPartitionedConvolver : juce::dsp::ProcessorBase
{
public:
    void prepare (const juce::dsp::ProcessSpec & spec) override;
    void process (const juce::dsp::ProcessContextReplacing<float> & replacing) override;
    void reset () override;
    void LoadImpulseResponse (juce::dsp::AudioBlock<float> ir_block, double sample_rate);

private:
    juce::dsp::ProcessSpec process_spec_;
    UniformPartitionedConvolver uniform_partitioned_convolver_;

    int circular_buffer_size_;

    juce::AudioBuffer<float> held_input_buffer_;

    juce::AudioBuffer<float> convolution_result_;
    CircularBuffer circular_buffer_ {convolution_result_};

    bool primary_in_use_ = false;
    TimeDistributedUPC primary_tdupc_;

    bool secondary_in_use_ = false;
    TimeDistributedUPC secondary_tdupc_;
};