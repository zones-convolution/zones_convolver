#pragma once

namespace zones
{
class UPCWorker
{
public:
    UPCWorker (const juce::dsp::ProcessSpec & spec,
               int partition_size_blocks,
               juce::dsp::AudioBlock<const float> ir_segment);

    void Reset ();

private:
    std::vector<ComplexBuffer> filter_partitions_;
};
}