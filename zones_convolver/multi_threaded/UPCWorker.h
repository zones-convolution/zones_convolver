#pragma once

#include <juce_dsp/juce_dsp.h>
namespace zones
{
class UPCWorker
{
public:
    UPCWorker (const juce::dsp::ProcessSpec & spec,
               int partition_size_blocks,
               juce::dsp::AudioBlock<const float> ir_segment);

    void Process (const juce::dsp::ProcessContextReplacing<float> & replacing);

    void Reset ();

private:
    void SignalDoConvolution ();

    juce::dsp::AudioBlock<const float> GetProcessedSamplesAndSwitchBuffer ();

    int partition_size_samples_ {};

    juce::AudioBuffer<float> scratch_buffer_ {};

    UniformPartitionedConvolver upc_;

    juce::SpinLock spin_lock_;
    std::array<juce::AudioBuffer<float>, 2> double_buffers_;
    int worker_buffer_index_ = 0;
};
}