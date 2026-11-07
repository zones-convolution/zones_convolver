#pragma once

namespace zones
{
class UPCWorker
{
public:
    UPCWorker (const juce::dsp::ProcessSpec & spec,
               int partition_size_blocks,
               juce::dsp::AudioBlock<const float> ir_segment);

    void SignalDoConvolution (const juce::dsp::AudioBlock<const float> & new_samples);

    juce::dsp::AudioBlock<const float> GetProcessedSamplesAndSwitchBuffer ();

    void Reset ();

private:
    std::vector<ComplexBuffer> filter_partitions_;
};
}