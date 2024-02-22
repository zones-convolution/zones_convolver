#include "TimeDistributedNUPC.h"

std::vector<TimeDistributedNUPC::PartitionLayout>
TimeDistributedNUPC::GetPartitionScheme (int block_size, int ir_num_samples)
{
    std::vector<PartitionLayout> partition_scheme;

    static constexpr auto kMaxNumPartitionsInUPC = 8;
    auto max_num_samples_in_upc = kMaxNumPartitionsInUPC * block_size;
    auto num_samples_in_upc = std::min (ir_num_samples, max_num_samples_in_upc);
    auto num_partitions_in_upc =
        GetNumPartitionsRequiredForSegment (block_size, num_samples_in_upc);
    partition_scheme.emplace_back (
        PartitionLayout {.partition_size_blocks = 1, .num_partitions = num_partitions_in_upc});

    auto remaining_samples_to_convolve = ir_num_samples - num_samples_in_upc;
    if (remaining_samples_to_convolve > 0)
    {
        static constexpr auto kMaxNumPartitionsPrimary = 8;
        const auto kPrimaryPartitionSizeBlocks = 4;
        const auto kPrimaryPartitionSizeSamples = kPrimaryPartitionSizeBlocks * block_size;

        auto num_samples_primary = std::min (
            remaining_samples_to_convolve, kPrimaryPartitionSizeSamples * kMaxNumPartitionsPrimary);

        auto num_partitions_primary =
            GetNumPartitionsRequiredForSegment (kPrimaryPartitionSizeSamples, num_samples_primary);

        partition_scheme.emplace_back (
            PartitionLayout {.partition_size_blocks = kPrimaryPartitionSizeBlocks,
                             .num_partitions = num_partitions_primary});

        remaining_samples_to_convolve -= num_samples_primary;
    }

    if (remaining_samples_to_convolve > 0)
    {
        const auto kSecondaryPartitionSizeBlocks = 16;
        const auto kSecondaryPartitionSizeSamples = kSecondaryPartitionSizeBlocks * block_size;
        auto num_partitions_secondary = GetNumPartitionsRequiredForSegment (
            kSecondaryPartitionSizeSamples, remaining_samples_to_convolve);
        partition_scheme.emplace_back (
            PartitionLayout {.partition_size_blocks = kSecondaryPartitionSizeBlocks,
                             .num_partitions = num_partitions_secondary});
    }

    return partition_scheme;
}

TimeDistributedNUPC::TimeDistributedNUPC (juce::dsp::AudioBlock<const float> ir_block,
                                          const juce::dsp::ProcessSpec & spec)
{
    const auto num_channels = static_cast<int> (spec.numChannels);
    const auto block_size = static_cast<int> (spec.maximumBlockSize);
    const auto ir_num_samples = static_cast<int> (ir_block.getNumSamples ());

    jassert (ir_num_samples > 0);

    auto partition_scheme = GetPartitionScheme (block_size, ir_num_samples);

    const auto upc_layout = partition_scheme [0];
    auto offset = upc_layout.GetSubConvolverSizeSamples ();
    auto upc_ir_segment = ir_block.getSubBlock (0, offset);
    upc_ = std::make_unique<UniformPartitionedConvolver> (spec, upc_ir_segment);

    partition_scheme.erase (partition_scheme.begin ());
    auto num_tdupc = partition_scheme.size ();

    for (auto tdupc_index = 0; tdupc_index < num_tdupc; ++tdupc_index)
    {
        const auto & layout = partition_scheme [tdupc_index];
        auto tdupc_size = layout.GetSubConvolverSizeSamples ();
        auto ir_segment = ir_block.getSubBlock (offset, tdupc_size);
        tdupcs_.emplace_back (spec, layout.partition_size_blocks, ir_segment);
        sub_convolver_delays_.push_back (offset - (layout.partition_size_blocks * block_size));
        offset += tdupc_size;
    }

    if (num_tdupc > 0)
    {
        auto sub_convolver_delay_size = sub_convolver_delays_.back () + block_size;

        sub_convolver_delay_buffer_.setSize (num_channels, sub_convolver_delay_size);
        sub_convolver_delay_buffer_.clear ();

        process_buffer_.setSize (num_channels, block_size);
        process_buffer_.clear ();
    }
}

void TimeDistributedNUPC::Process (const juce::dsp::ProcessContextReplacing<float> & replacing)
{
    auto output_block = replacing.getOutputBlock ();
    auto block_size = output_block.getNumSamples ();

    juce::dsp::AudioBlock<float> process_block {process_buffer_};
    auto num_tdupc = tdupcs_.size ();

    for (auto tdupc_index = 0; tdupc_index < num_tdupc; ++tdupc_index)
    {
        process_block.copyFrom (output_block);
        tdupcs_ [tdupc_index].Process (process_block);

        auto delayed_block = circular_buffer_.GetNext (sub_convolver_delays_ [tdupc_index], false);
        delayed_block.AddFrom (process_block);
    }

    upc_->Process (replacing);

    if (num_tdupc > 0)
    {
        auto delayed_result = circular_buffer_.GetNext (block_size, true).GetSubBlock (block_size);
        delayed_result.AddTo (output_block);
        delayed_result.Clear ();
    }
}

void TimeDistributedNUPC::Reset ()
{
    sub_convolver_delay_buffer_.clear ();
    process_buffer_.clear ();
    upc_->Reset ();
    for (auto & tdupc : tdupcs_)
        tdupc.Reset ();
}
