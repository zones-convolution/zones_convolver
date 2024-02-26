#include "TimeDistributedNUPC.h"

TimeDistributedNUPC::TimeDistributedNUPC (juce::dsp::AudioBlock<const float> ir_block,
                                          const juce::dsp::ProcessSpec & spec)
{
    const auto num_channels = static_cast<int> (spec.numChannels);
    const auto block_size = static_cast<int> (spec.maximumBlockSize);
    const auto ir_num_samples = static_cast<int> (ir_block.getNumSamples ());

    jassert (ir_num_samples > 0);

    auto partition_scheme = GetPartitionScheme (kGarciaResults, block_size, ir_num_samples);

    const auto upc_layout = partition_scheme.front ();
    auto offset = upc_layout.GetSubConvolverSizeSamples (block_size);
    auto upc_ir_segment = ir_block.getSubBlock (0, offset);
    upc_ = std::make_unique<UniformPartitionedConvolver> (spec, upc_ir_segment);

    partition_scheme.erase (partition_scheme.begin ());
    auto num_tdupc = partition_scheme.size ();

    for (auto tdupc_index = 0; tdupc_index < num_tdupc; ++tdupc_index)
    {
        const auto & layout = partition_scheme [tdupc_index];
        auto tdupc_size = layout.GetSubConvolverSizeSamples (block_size);
        auto ir_segment =
            ir_block.getSubBlock (offset, std::min (tdupc_size, ir_num_samples - offset));
        auto partition_size = layout.partition_size_blocks * block_size;
        tdupcs_.push_back (std::make_shared<TimeDistributedUPCMulti> (
            spec, layout.partition_size_blocks, ir_segment));
        auto sub_convolver_delay = offset - (2 * partition_size) + block_size;
        sub_convolver_delays_.push_back (sub_convolver_delay);
        offset += tdupc_size;
    }

    if (num_tdupc > 0)
    {
        sub_convolver_delay_buffer_.setSize (num_channels, sub_convolver_delays_.back ());
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
        tdupcs_ [tdupc_index]->Process (process_block);

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
    for (auto tdupc : tdupcs_)
        tdupc->Reset ();
}
