#pragma once

#include "TimeDistributedUPC.h"

namespace zones
{
class TimeDistributedUPCMulti
{
public:
    TimeDistributedUPCMulti (const juce::dsp::ProcessSpec & spec,
                             int partition_size_blocks,
                             juce::dsp::AudioBlock<const float> ir_segment);

    void Process (const juce::dsp::ProcessContextReplacing<float> & replacing);
    void Reset ();

private:
    std::vector<TimeDistributedUPC> tdupcs_;
    std::vector<ComplexBuffer> filter_partitions_;
};
}