#pragma once

#include "TimeDistributedUPCMulti.h"

#include <juce_dsp/juce_dsp.h>
#include <zones_convolver/PartitionSchemes.h>
#include <zones_convolver/UniformPartitionedConvolver.h>
#include <zones_convolver/util/Partitioning.h>

namespace zones
{
class TimeDistributedNUPC
{
public:
    TimeDistributedNUPC (juce::dsp::AudioBlock<const float> ir_block,
                         const juce::dsp::ProcessSpec & spec);
    void Process (const juce::dsp::ProcessContextReplacing<float> & replacing);
    void Reset ();

private:
    std::unique_ptr<UniformPartitionedConvolver> upc_;
    std::vector<std::shared_ptr<TimeDistributedUPCMulti>> tdupcs_;

    int max_block_size_ = 0;
    int num_samples_collected_ = 0;

    juce::AudioBuffer<float> saved_input_buffer_;

    std::vector<int> sub_convolver_delays_;

    juce::AudioBuffer<float> process_buffer_;
    juce::AudioBuffer<float> sub_convolver_delay_buffer_;
    CircularBuffer result_buffer_ {sub_convolver_delay_buffer_};
};
}