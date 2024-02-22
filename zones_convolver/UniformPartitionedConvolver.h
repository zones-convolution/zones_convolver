#pragma once

#include "util/CircularBuffer.h"
#include "util/ComplexBuffer.h"
#include "util/FrequencyDelayLine.h"

#include <juce_dsp/juce_dsp.h>

// class UniformPartitionedConvolver : juce::dsp::ProcessorBase
//{
// public:
//     void prepare (const juce::dsp::ProcessSpec & spec) override;
//     void process (const juce::dsp::ProcessContextReplacing<float> & replacing) override;
//     void reset () override;
//     void LoadImpulseResponse (juce::dsp::AudioBlock<float> ir_block, double sample_rate);
//
// private:
//     juce::dsp::ProcessSpec process_spec_;
//
//     std::size_t fft_size_;
//     std::unique_ptr<juce::dsp::FFT> fft_;
//     std::size_t num_samples_to_discard_ = 0;
//
//     std::unique_ptr<FrequencyDelayLine> frequency_delay_line_;
//     std::unique_ptr<ComplexBuffer> convolved_output_;
//     std::vector<ComplexBuffer> filter_partitions_;
//
//     juce::AudioBuffer<float> saved_inputs_;
//     std::unique_ptr<CircularBuffer> circular_buffer_;
// };

class UniformPartitionedConvolver
{
public:
    UniformPartitionedConvolver (const juce::dsp::ProcessSpec & spec,
                                 juce::dsp::AudioBlock<const float> ir_segment);
    void Process (const juce::dsp::ProcessContextReplacing<float> & replacing);
    void Reset ();

private:
};
