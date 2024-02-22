#include "UniformPartitionedConvolver.h"

UniformPartitionedConvolver::UniformPartitionedConvolver (
    const juce::dsp::ProcessSpec & spec,
    juce::dsp::AudioBlock<const float> ir_segment)
{
    const auto num_channels = static_cast<int> (spec.numChannels);
    const auto ir_num_channels = static_cast<int> (ir_segment.getNumChannels ());
    const auto block_size = static_cast<int> (spec.maximumBlockSize);

    fft_size_ = juce::nextPowerOfTwo (block_size * 2);

    auto fft_order = std::log2 (fft_size_);
    fft_ = std::make_unique<juce::dsp::FFT> (fft_order);

    num_samples_to_discard_ = fft_size_ - block_size;

    const auto ir_num_samples = static_cast<int> (ir_segment.getNumSamples ());
    auto num_partitions = GetNumPartitionsRequiredForSegment (block_size, ir_num_samples);

    saved_inputs_.setSize (num_channels, fft_size_);
    saved_inputs_.clear ();

    for (auto partition_index = 0; partition_index < num_partitions; ++partition_index)
    {
        auto partition_offset = partition_index * block_size;
        auto partition_length = std::min (block_size, ir_num_samples - partition_offset);
        auto partition_block = ir_segment.getSubBlock (partition_offset, partition_length);

        ComplexBuffer filter_partition {static_cast<std::size_t> (fft_size_),
                                        static_cast<std::size_t> (num_channels)};
        filter_partition.Clear ();
        auto filter_block = filter_partition.GetContinuousBlock ();
        for (auto channel_index = 0; channel_index < num_channels; ++channel_index)
        {
            filter_block.getSingleChannelBlock (channel_index)
                .copyFrom (partition_block.getSingleChannelBlock (channel_index % ir_num_channels));
            fft_->performRealOnlyForwardTransform (filter_block.getChannelPointer (channel_index),
                                                   true);
        }

        filter_partitions_.emplace_back (std::move (filter_partition));
    }

    frequency_delay_line_ =
        std::make_unique<FrequencyDelayLine> (num_channels, num_partitions, fft_size_);

    convolved_output_ = std::make_unique<ComplexBuffer> (fft_size_, num_channels);
    convolved_output_->Clear ();
}

void UniformPartitionedConvolver::Process (
    const juce::dsp::ProcessContextReplacing<float> & replacing)
{
    auto output_block = replacing.getOutputBlock ();
    auto num_channels = static_cast<int> (output_block.getNumChannels ());
    auto num_partitions = static_cast<int> (filter_partitions_.size ());
    auto num_samples = static_cast<int> (output_block.getNumSamples ());

    auto & first_fdl_block = frequency_delay_line_->GetNextBlock ();
    first_fdl_block.Clear ();
    auto continuous_fdl_block = first_fdl_block.GetContinuousBlock ();

    circular_buffer_.GetNext (0).CopyFrom (output_block);
    circular_buffer_.GetNext (num_samples).CopyTo (continuous_fdl_block);

    for (auto channel_index = 0u; channel_index < num_channels; ++channel_index)
        fft_->performRealOnlyForwardTransform (
            continuous_fdl_block.getChannelPointer (channel_index), true);

    auto & first_partition = filter_partitions_ [0];
    convolved_output_->ComplexMultiplyFrom (first_fdl_block, first_partition);

    for (auto partition_index = 1; partition_index < num_partitions; ++partition_index)
    {
        auto & previous_fdl_block = frequency_delay_line_->GetBlockWithOffset (partition_index);
        auto & partition = filter_partitions_ [partition_index];

        convolved_output_->ComplexMultiplyAccumulateFrom (previous_fdl_block, partition);
    }

    auto continuous_output_block = convolved_output_->GetContinuousBlock ();
    for (auto channel_index = 0u; channel_index < num_channels; ++channel_index)
        fft_->performRealOnlyInverseTransform (
            continuous_output_block.getChannelPointer (channel_index));

    output_block.copyFrom (
        continuous_output_block.getSubBlock (num_samples_to_discard_, num_samples));
}

void UniformPartitionedConvolver::Reset ()
{
    frequency_delay_line_->Clear ();
    saved_inputs_.clear ();
    convolved_output_->Clear ();
}
