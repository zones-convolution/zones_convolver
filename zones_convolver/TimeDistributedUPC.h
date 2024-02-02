#pragma once

#include "util/ComplexBuffer.h"
#include "util/DecompositionSchedule.h"
#include "util/FrequencyDelayLine.h"

#include <juce_dsp/juce_dsp.h>

class TimeDistributedUPC
{
public:
    void Prepare (const juce::dsp::ProcessSpec & spec,
                  int partition_size_samples,
                  juce::dsp::AudioBlock<float> ir_segment);
    void Process (const juce::dsp::ProcessContextReplacing<float> & replacing);

private:
    void PrepareFilterPartitions (juce::dsp::AudioBlock<float> ir_segment,
                                  int partition_size_samples,
                                  const juce::dsp::ProcessSpec & spec);

    juce::dsp::ProcessSpec spec_;
    int partition_size_samples_;
    int fft_num_points_;

    int num_phases_;
    int phase_;

    std::vector<ComplexBuffer> filter_partitions_;
    std::unique_ptr<FrequencyDelayLine> frequency_delay_line_;

    DecompositionSchedule::DecompositionPlan forward_decomposition_plan_;
    DecompositionSchedule::DecompositionPlan inverse_decomposition_plan_;

    std::unique_ptr<StageBuffers> stage_buffers_;
};
