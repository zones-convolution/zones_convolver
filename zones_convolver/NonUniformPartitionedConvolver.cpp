#include "NonUniformPartitionedConvolver.h"

void NonUniformPartitionedConvolver::prepare (const juce::dsp::ProcessSpec & spec)
{
    process_spec_ = spec;

    held_input_buffer_.setSize (spec.numChannels, spec.maximumBlockSize);
    held_input_buffer_.clear ();

    cb_hold_buffer_.setSize (spec.numChannels, spec.maximumBlockSize);
    cb_hold_buffer_.clear ();

    primary_tdupc_.clear ();
    secondary_tdupc_.clear ();

    for (auto channel_index = 0; channel_index < spec.numChannels; ++channel_index)
    {
        primary_tdupc_.push_back (TimeDistributedUPC ());
        secondary_tdupc_.push_back (TimeDistributedUPC ());
    }
}

void NonUniformPartitionedConvolver::process (
    const juce::dsp::ProcessContextReplacing<float> & replacing)
{
    auto output_block = replacing.getOutputBlock ();
    auto num_channels = output_block.getNumChannels ();
    auto block_size = process_spec_.maximumBlockSize;

    juce::dsp::AudioBlock<float> held_input_block {held_input_buffer_};
    held_input_block.copyFrom (output_block);

    auto next_block = circular_buffer_.GetNext (block_size, true).GetSubBlock (block_size);

    for (auto channel_index = 0; channel_index < num_channels; ++channel_index)
    {
        auto channel_block = output_block.getSingleChannelBlock (channel_index);
        juce::dsp::ProcessContextReplacing<float> channel_context_replacing {channel_block};

        if (secondary_in_use_)
        {
            secondary_tdupc_ [channel_index].Process (channel_context_replacing);
            juce::dsp::AudioBlock<float> (cb_hold_buffer_)
                .getSingleChannelBlock (channel_index)
                .copyFrom (channel_block);
        }

        if (primary_in_use_)
        {
            channel_block.copyFrom (held_input_block.getSingleChannelBlock (channel_index));
            primary_tdupc_ [channel_index].Process (channel_context_replacing);
        }
    }

    if (secondary_in_use_)
    {
        auto delayed_block = circular_buffer_.GetNext (circular_buffer_size_, false);
        delayed_block.AddFrom (cb_hold_buffer_);
    }

    if (primary_in_use_)
        next_block.AddFrom (output_block);

    output_block.copyFrom (held_input_block);
    uniform_partitioned_convolver_.process (replacing);

    next_block.AddTo (output_block);
    next_block.Clear ();
}

void NonUniformPartitionedConvolver::reset ()
{
    uniform_partitioned_convolver_.reset ();
}

void NonUniformPartitionedConvolver::LoadImpulseResponse (juce::dsp::AudioBlock<float> ir_block,
                                                          double sample_rate)
{
    primary_in_use_ = false;
    secondary_in_use_ = false;

    // {1024: 8, 4096: 8, 16384: ... }
    // {b: 8, 4b: 8, 16b: ... }

    auto ir_length_samples = ir_block.getNumSamples ();
    auto block_size = process_spec_.maximumBlockSize;
    auto num_channels = process_spec_.numChannels;

    auto num_ir_channels = ir_block.getNumChannels ();

    static constexpr auto kMaxNumBlocksInUPC = 8;
    auto max_num_samples_in_upc = kMaxNumBlocksInUPC * block_size;
    auto num_samples_in_upc =
        std::min (static_cast<int> (ir_length_samples), static_cast<int> (max_num_samples_in_upc));
    auto upc_block = ir_block.getSubBlock (0, num_samples_in_upc);

    uniform_partitioned_convolver_.prepare (process_spec_);
    uniform_partitioned_convolver_.LoadImpulseResponse (upc_block, sample_rate);

    auto remaining_samples_to_convolve = ir_length_samples - num_samples_in_upc;
    auto offset = num_samples_in_upc;

    if (remaining_samples_to_convolve > 0)
    {
        static constexpr auto kMaxNumBlocksInPrimaryTDUPC = 8;
        const auto kPrimaryTDUPCPartitionSize = 4 * block_size;

        auto num_samples_primary_TDUPC =
            std::min (static_cast<int> (remaining_samples_to_convolve),
                      static_cast<int> (kMaxNumBlocksInPrimaryTDUPC * kPrimaryTDUPCPartitionSize));

        auto primary_TDUPC_block = ir_block.getSubBlock (offset, num_samples_primary_TDUPC);

        for (auto channel_index = 0; channel_index < process_spec_.numChannels; ++channel_index)
        {
            primary_tdupc_ [channel_index].Prepare (
                process_spec_,
                kPrimaryTDUPCPartitionSize,
                primary_TDUPC_block.getSingleChannelBlock (channel_index % num_ir_channels));
        }

        remaining_samples_to_convolve -= num_samples_primary_TDUPC;

        offset += num_samples_primary_TDUPC;

        primary_in_use_ = true;
    }

    const auto kSecondaryTDUPCPartitionSize = 16 * block_size;

    if (remaining_samples_to_convolve > 0)
    {
        auto secondary_TDUPC_block = ir_block.getSubBlock (offset, remaining_samples_to_convolve);

        for (auto channel_index = 0; channel_index < process_spec_.numChannels; ++channel_index)
        {
            secondary_tdupc_ [channel_index].Prepare (
                process_spec_,
                kSecondaryTDUPCPartitionSize,
                secondary_TDUPC_block.getSingleChannelBlock (channel_index % num_ir_channels));
        }

        secondary_in_use_ = true;
    }

    if (secondary_in_use_)
    {
        auto circular_buffer_size = offset - (2 * kSecondaryTDUPCPartitionSize);
        circular_buffer_size_ = circular_buffer_size;
        convolution_result_.setSize (process_spec_.numChannels, circular_buffer_size + block_size);
    }
    else
    {
        convolution_result_.setSize (process_spec_.numChannels, block_size);
        circular_buffer_size_ = 0;
    }

    convolution_result_.clear ();
}
