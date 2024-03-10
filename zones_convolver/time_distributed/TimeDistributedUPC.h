#pragma once

#include <juce_dsp/juce_dsp.h>
#include <zones_convolver/util/ComplexBuffer.h>
#include <zones_convolver/util/DecompositionSchedule.h>
#include <zones_convolver/util/FrequencyDelayLine.h>
#include <zones_convolver/util/Partitioning.h>

namespace zones
{
class TimeDistributedUPC
{
public:
    TimeDistributedUPC (const juce::dsp::ProcessSpec & spec,
                        int partition_size_blocks,
                        const std::vector<ComplexBuffer> & filter_partitions,
                        int filter_channel,
                        int transform_offset,
                        int num_decompositions);
    void Process (const juce::dsp::ProcessContextReplacing<float> & replacing);
    void Reset ();

private:
    int partition_size_samples_;
    int fft_num_points_;

    int num_phases_;
    int phase_;
    int num_decompositions_;
    int num_partitions_;
    int transform_offset_;

    const std::vector<ComplexBuffer> & filter_partitions_;
    int filter_channel_;
    std::unique_ptr<FrequencyDelayLine> frequency_delay_line_;
    std::unique_ptr<StageBuffers> stage_buffers_;
    juce::AudioBuffer<float> previous_tail_;
};
}