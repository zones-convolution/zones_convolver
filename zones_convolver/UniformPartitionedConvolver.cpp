#include "UniformPartitionedConvolver.h"

UniformPartitionedConvolver::UniformPartitionedConvolver (
    const juce::dsp::ProcessSpec & spec,
    juce::dsp::AudioBlock<const float> ir_segment)
{
    const auto num_channels = static_cast<int> (spec.numChannels);
    const auto ir_num_channels = static_cast<int> (ir_segment.getNumChannels ());
    partition_size_ = static_cast<int> (spec.maximumBlockSize);

    fft_size_ = juce::nextPowerOfTwo (partition_size_ * 2);

    auto fft_order = std::log2 (fft_size_);
    fft_ = std::make_unique<juce::dsp::FFT> (fft_order);

    num_samples_to_discard_ = fft_size_ - partition_size_;

    const auto ir_num_samples = static_cast<int> (ir_segment.getNumSamples ());
    auto num_partitions = GetNumPartitionsRequiredForSegment (partition_size_, ir_num_samples);

    saved_inputs_.setSize (num_channels, fft_size_);

    for (auto partition_index = 0; partition_index < num_partitions; ++partition_index)
    {
        auto partition_offset = partition_index * partition_size_;
        auto partition_length = std::min (partition_size_, ir_num_samples - partition_offset);
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
    fdl_convolved_output_ = std::make_unique<ComplexBuffer> (fft_size_, num_channels);
    ifft_buffer_ = std::make_unique<ComplexBuffer> (fft_size_, num_channels);

    Reset ();
}

void UniformPartitionedConvolver::Process (
    const juce::dsp::ProcessContextReplacing<float> & replacing)
{
    auto output_block = replacing.getOutputBlock ();
    auto num_channels = static_cast<int> (output_block.getNumChannels ());
    auto num_partitions = static_cast<int> (filter_partitions_.size ());
    auto block_size = static_cast<int> (output_block.getNumSamples ());

    auto num_samples_processed = 0;
    while (num_samples_processed < block_size)
    {
        if (num_samples_collected_ == 0)
        {
            frequency_delay_line_->GetNextBlock ();
            fdl_convolved_output_->Clear ();

            // Could start from index 2 and then not do the clear above, CMUL on
            // partition[1]
            for (auto partition_index = 1; partition_index < num_partitions; ++partition_index)
            {
                auto & previous_fdl_block =
                    frequency_delay_line_->GetBlockWithOffset (partition_index);
                auto & partition = filter_partitions_ [partition_index];

                fdl_convolved_output_->ComplexMultiplyAccumulateFrom (previous_fdl_block,
                                                                      partition);
            }

            circular_buffer_.GetNext (partition_size_, true);
        }

        auto num_samples_to_process =
            std::min (block_size - num_samples_processed, partition_size_ - num_samples_collected_);

        auto & first_fdl_block = frequency_delay_line_->GetBlockWithOffset (0);
        first_fdl_block.Clear ();
        auto continuous_fdl_block = first_fdl_block.GetContinuousBlock ();

        auto input_to_copy =
            output_block.getSubBlock (num_samples_processed, num_samples_to_process);

        circular_buffer_.GetNext (num_samples_to_discard_ + num_samples_collected_, false)
            .CopyFrom (input_to_copy);
        circular_buffer_.GetNext (0, false).CopyTo (continuous_fdl_block);

        for (auto channel_index = 0u; channel_index < num_channels; ++channel_index)
            fft_->performRealOnlyForwardTransform (
                continuous_fdl_block.getChannelPointer (channel_index), true);

        auto & first_partition = filter_partitions_ [0];

        ifft_buffer_->ComplexMultiplyFrom (first_fdl_block, first_partition);
        ifft_buffer_->AddFrom (*fdl_convolved_output_);

        auto continuous_ifft_block = ifft_buffer_->GetContinuousBlock ();
        for (auto channel_index = 0u; channel_index < num_channels; ++channel_index)
            fft_->performRealOnlyInverseTransform (
                continuous_ifft_block.getChannelPointer (channel_index));

        auto offset_output_block =
            output_block.getSubBlock (num_samples_processed, num_samples_to_process);
        offset_output_block.copyFrom (continuous_ifft_block.getSubBlock (
            num_samples_to_discard_ + num_samples_collected_, num_samples_to_process));

        num_samples_processed += num_samples_to_process;
        num_samples_collected_ =
            (num_samples_collected_ + num_samples_to_process) % partition_size_;
    }
}

void UniformPartitionedConvolver::Reset ()
{
    saved_inputs_.clear ();

    num_samples_collected_ = 0;

    frequency_delay_line_->Clear ();
    fdl_convolved_output_->Clear ();
    ifft_buffer_->Clear ();
}
