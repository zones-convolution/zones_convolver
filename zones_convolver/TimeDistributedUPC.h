#pragma once

#include "util/ComplexBuffer.h"
#include "util/DecompositionSchedule.h"
#include "util/FrequencyDelayLine.h"
#include "util/Partitioning.h"

#include <juce_dsp/juce_dsp.h>

class TimeDistributedUPC
{
public:
    TimeDistributedUPC (const juce::dsp::ProcessSpec & spec,
                        int partition_size_blocks,
                        juce::dsp::AudioBlock<const float> ir_segment);
    void Process (const juce::dsp::ProcessContextReplacing<float> & replacing);
    void Reset ();

private:
    void PrepareFilterPartitions (juce::dsp::AudioBlock<const float> ir_segment,
                                  int partition_size_samples);

    int partition_size_samples_;
    int fft_num_points_;

    int num_phases_;
    int phase_;
    int num_decompositions_;
    int num_partitions_;

    std::vector<ComplexBuffer> filter_partitions_;
    std::unique_ptr<FrequencyDelayLine> frequency_delay_line_;
    std::unique_ptr<StageBuffers> stage_buffers_;
    juce::AudioBuffer<float> previous_tail_;
};
