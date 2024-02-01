#include "TimeDistributedUPC.h"

void TimeDistributedUPC::Prepare (const juce::dsp::ProcessSpec & spec,
                                  int partition_size_samples,
                                  juce::dsp::AudioBlock<float> ir_segment)
{
    spec_ = spec;
    partition_size_samples_ = partition_size_samples;

    auto num_blocks_in_partition = partition_size_samples_ / spec_.maximumBlockSize;
    auto maximal_decompositions = std::log2 (num_blocks_in_partition) - 1;

    forward_decomposition_plan_ = DecompositionSchedule::CreateForwardDecompositionPlan (
        num_blocks_in_partition, num_blocks_in_partition, maximal_decompositions);

    inverse_decomposition_plan_ = DecompositionSchedule::CreateInverseDecompositionPlan (
        num_blocks_in_partition, num_blocks_in_partition, maximal_decompositions);

    fft_num_points_ = 2 * partition_size_samples_;
    stage_buffers_ = std::make_unique<StageBuffers> (fft_num_points_);

    PrepareFilterPartitions (ir_segment, partition_size_samples, spec);
}

void TimeDistributedUPC::Process (const juce::dsp::ProcessContextReplacing<float> & replacing)
{
}

void TimeDistributedUPC::PrepareFilterPartitions (juce::dsp::AudioBlock<float> ir_segment,
                                                  int partition_size_samples,
                                                  const juce::dsp::ProcessSpec & spec)
{
    auto filter_size = ir_segment.getNumSamples ();
    auto num_partitions = static_cast<int> (
        std::ceil (static_cast<float> (filter_size) / static_cast<float> (partition_size_samples)));

    for (auto partition_index = 0; partition_index < num_partitions; ++partition_index)
    {
        auto filter_partition_block = ir_segment.getSubBlock (
            partition_index * partition_size_samples, partition_size_samples);
        auto filter_partition = ComplexBuffer (fft_num_points_, 1);
        filter_partition.Clear ();
        filter_partition.CopyFromAudioBlock (filter_partition_block);

        filter_partitions_.emplace_back (std::move (filter_partition));
    }
}
