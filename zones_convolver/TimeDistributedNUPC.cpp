#include "TimeDistributedNUPC.h"

int GetNumPartitionsRequiredForSubConvolver (int partition_size, int sub_convolver_num_samples)
{
    return static_cast<int> (std::ceil (static_cast<float> (sub_convolver_num_samples) /
                                        static_cast<float> (partition_size)));
}

std::vector<TimeDistributedNUPC::PartitionLayout>
TimeDistributedNUPC::GetPartitionScheme (int block_size, int ir_num_samples)
{
    std::vector<PartitionLayout> partition_scheme;

    static constexpr auto kMaxNumPartitionsInUPC = 8;
    auto max_num_samples_in_upc = kMaxNumPartitionsInUPC * block_size;
    auto num_samples_in_upc = std::min (ir_num_samples, max_num_samples_in_upc);
    auto num_partitions_in_upc =
        GetNumPartitionsRequiredForSubConvolver (block_size, num_samples_in_upc);
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

        auto num_partitions_primary = GetNumPartitionsRequiredForSubConvolver (
            kPrimaryPartitionSizeSamples, num_samples_primary);

        partition_scheme.emplace_back (
            PartitionLayout {.partition_size_blocks = kPrimaryPartitionSizeBlocks,
                             .num_partitions = num_partitions_primary});

        remaining_samples_to_convolve -= num_samples_primary;
    }

    if (remaining_samples_to_convolve > 0)
    {
        const auto kSecondaryPartitionSizeBlocks = 16;
        const auto kSecondaryPartitionSizeSamples = kSecondaryPartitionSizeBlocks * block_size;
        auto num_partitions_secondary = GetNumPartitionsRequiredForSubConvolver (
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
    const auto block_size = static_cast<int> (spec.maximumBlockSize);
    const auto ir_num_samples = static_cast<int> (ir_block.getNumSamples ());

    jassert (ir_num_samples > 0);

    auto partition_scheme = GetPartitionScheme (block_size, ir_num_samples);

    const auto upc_layout = partition_scheme [0];
    auto offset = upc_layout.GetSubConvolverSizeSamples ();
    auto upc_ir_segment = ir_block.getSubBlock (0, offset);
    upc_ = std::make_unique<UniformPartitionedConvolver> (spec, upc_ir_segment);

    partition_scheme.erase (partition_scheme.begin ());
    num_tdupc_ = partition_scheme.size ();

    for (auto tdupc_index = 0; tdupc_index < num_tdupc_; ++tdupc_index)
    {
        const auto & layout = partition_scheme [tdupc_index];
        auto tdupc_size = layout.GetSubConvolverSizeSamples ();
        auto ir_segment = ir_block.getSubBlock (offset, tdupc_size);
        tdupcs_.emplace_back (spec, layout.partition_size_blocks, ir_segment);
        sub_convolver_delays_.push_back (offset - (layout.partition_size_blocks * block_size));
        offset += tdupc_size;
    }

    if (num_tdupc_ > 0)
    {
        auto sub_convolver_delay_size = sub_convolver_delays_.back () + block_size;
        sub_convolver_delay_buffer_.setSize (spec.numChannels, sub_convolver_delay_size);
        sub_convolver_delay_buffer_.clear ();
    }
}

void TimeDistributedNUPC::Process (const juce::dsp::ProcessContextReplacing<float> & replacing)
{
    /*
     * int num_tdupc_;
     * vector<int> clerances_; // OFFSET?
     * juce::AudioBuffer<float> process_buffer_;
     *
     * for (auto conv_index = 0; conv_index < num_tdupc_; ++ conv_index) {
     *      copy input to process_buffer_
     *      process tdupc_[conv_index]
     *
     *       auto clerance = clerances_[conv_index];
     *       write process buffer into CB with clerance
     * }
     */

    // Process UPC
    // if(num_tdupc_ > 0) {
    //      Add CB to output
    //      clear segment of circular buffer
    //      progress circular buffer
    // }
}

void TimeDistributedNUPC::Reset ()
{
    sub_convolver_delay_buffer_.clear ();
    upc_->Reset ();
    for (auto & tdupc : tdupcs_)
        tdupc.Reset ();
}
