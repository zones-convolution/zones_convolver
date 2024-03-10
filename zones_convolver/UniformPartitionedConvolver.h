#pragma once

#include "util/CircularBuffer.h"
#include "util/ComplexBuffer.h"
#include "util/FrequencyDelayLine.h"

#include <juce_dsp/juce_dsp.h>

namespace zones
{
class UniformPartitionedConvolver
{
public:
    UniformPartitionedConvolver (const juce::dsp::ProcessSpec & spec,
                                 juce::dsp::AudioBlock<const float> ir_segment);
    void Process (const juce::dsp::ProcessContextReplacing<float> & replacing);
    void Reset ();

private:
    int fft_size_;
    std::unique_ptr<juce::dsp::FFT> fft_;
    int num_samples_to_discard_ = 0;
    int num_samples_collected_ = 0;
    int partition_size_ = 0;

    std::unique_ptr<FrequencyDelayLine> frequency_delay_line_;
    std::unique_ptr<ComplexBuffer> fdl_convolved_output_;
    std::unique_ptr<ComplexBuffer> ifft_buffer_;
    std::vector<ComplexBuffer> filter_partitions_;
    juce::AudioBuffer<float> saved_inputs_;
    CircularBuffer circular_buffer_ {saved_inputs_};
};
}