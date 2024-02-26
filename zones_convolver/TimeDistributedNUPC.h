#pragma once

#include "PartitionSchemes.h"
#include "TimeDistributedUPCMulti.h"
#include "UniformPartitionedConvolver.h"
#include "util/Partitioning.h"

#include <juce_dsp/juce_dsp.h>

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

    std::vector<int> sub_convolver_delays_;

    juce::AudioBuffer<float> process_buffer_;
    juce::AudioBuffer<float> sub_convolver_delay_buffer_;
    CircularBuffer circular_buffer_ {sub_convolver_delay_buffer_};
};
