#pragma once

#include "TimeDistributedUPCMulti.h"
#include "UniformPartitionedConvolver.h"

#include <juce_dsp/juce_dsp.h>

class TimeDistributedNUPC
{
public:
    TimeDistributedNUPC (juce::dsp::AudioBlock<const float> ir_block,
                         const juce::dsp::ProcessSpec & spec);
    void Process (const juce::dsp::ProcessContextReplacing<float> & replacing);
    void Reset ();

private:
    struct PartitionLayout
    {
        int partition_size_blocks;
        int num_partitions;

        [[nodiscard]] int GetSubConvolverSizeSamples () const
        {
            return partition_size_blocks * num_partitions;
        }
    };

    static std::vector<PartitionLayout> GetPartitionScheme (int block_size, int ir_num_samples);

    std::unique_ptr<UniformPartitionedConvolver> upc_;

    int num_tdupc_;
    std::vector<TimeDistributedUPCMulti> tdupcs_;
    std::vector<int> sub_convolver_delays_;

    juce::AudioBuffer<float> sub_convolver_delay_buffer_;
    CircularBuffer circular_buffer_ {sub_convolver_delay_buffer_};
};
