#include "TimeDistributedNUPC.h"

namespace zones
{
TimeDistributedNUPC::TimeDistributedNUPC (juce::dsp::AudioBlock<const float> ir_block,
                                          const juce::dsp::ProcessSpec & spec)
{
    max_block_size_ = static_cast<int> (spec.maximumBlockSize);
    const auto num_channels = static_cast<int> (spec.numChannels);
    const auto ir_num_samples = static_cast<int> (ir_block.getNumSamples ());

    jassert (ir_num_samples > 0);

    auto partition_scheme = GetPartitionScheme (kGarciaResults, max_block_size_, ir_num_samples);

    const auto upc_layout = partition_scheme.front ();
    auto offset = upc_layout.GetSubConvolverSizeSamples (max_block_size_);

    auto upc_ir_segment = ir_block.getSubBlock (0, offset);
    upc_ = std::make_unique<UniformPartitionedConvolver> (spec, upc_ir_segment);

    partition_scheme.erase (partition_scheme.begin ());
    auto num_tdupc = partition_scheme.size ();

    for (auto tdupc_index = 0; tdupc_index < num_tdupc; ++tdupc_index)
    {
        const auto & layout = partition_scheme [tdupc_index];
        auto tdupc_size = layout.GetSubConvolverSizeSamples (max_block_size_);
        auto ir_segment =
            ir_block.getSubBlock (offset, std::min (tdupc_size, ir_num_samples - offset));
        auto partition_size = layout.partition_size_blocks * max_block_size_;
        tdupcs_.push_back (std::make_shared<TimeDistributedUPCMulti> (
            spec, layout.partition_size_blocks, ir_segment));
        auto sub_convolver_delay = offset - (2 * partition_size) + max_block_size_;
        sub_convolver_delays_.push_back (sub_convolver_delay);
        offset += tdupc_size;
    }

    if (num_tdupc > 0)
    {
        saved_input_buffer_.setSize (num_channels, max_block_size_);
        sub_convolver_delay_buffer_.setSize (num_channels, sub_convolver_delays_.back ());
        process_buffer_.setSize (num_channels, max_block_size_);
    }

    Reset ();
}

void TimeDistributedNUPC::Process (const juce::dsp::ProcessContextReplacing<float> & replacing)
{
    juce::dsp::AudioBlock<float> saved_input_block {saved_input_buffer_};
    auto output_block = replacing.getOutputBlock ();
    auto block_size = static_cast<int> (output_block.getNumSamples ());

    juce::dsp::AudioBlock<float> process_block {process_buffer_};
    auto num_tdupc = tdupcs_.size ();

    auto num_samples_processed = 0;
    while (num_samples_processed < block_size)
    {
        auto num_samples_to_process =
            std::min (block_size - num_samples_processed, max_block_size_ - num_samples_collected_);

        auto sub_output_replacing =
            output_block.getSubBlock (num_samples_processed, num_samples_to_process);

        if (num_tdupc > 0)
        {
            if (num_samples_collected_ == 0)
            {
                for (auto tdupc_index = 0; tdupc_index < num_tdupc; ++tdupc_index)
                {
                    process_block.copyFrom (saved_input_block);
                    tdupcs_ [tdupc_index]->Process (process_block);

                    auto delayed_block =
                        result_buffer_.GetNext (sub_convolver_delays_ [tdupc_index], false);
                    delayed_block.AddFrom (process_block);
                }
                result_buffer_.GetNext (max_block_size_, true);
            }

            saved_input_block.getSubBlock (num_samples_collected_).copyFrom (sub_output_replacing);
        }

        upc_->Process (sub_output_replacing);

        if (num_tdupc > 0)
        {
            auto delayed_result = result_buffer_.GetNext (num_samples_collected_, false)
                                      .GetSubBlock (num_samples_to_process);
            delayed_result.AddTo (sub_output_replacing);
            delayed_result.Clear ();
        }

        num_samples_processed += num_samples_to_process;
        num_samples_collected_ =
            (num_samples_collected_ + num_samples_to_process) % max_block_size_;
    }
}

void TimeDistributedNUPC::Reset ()
{
    num_samples_collected_ = 0;
    saved_input_buffer_.clear ();
    sub_convolver_delay_buffer_.clear ();
    process_buffer_.clear ();
    upc_->Reset ();
    for (auto tdupc : tdupcs_)
        tdupc->Reset ();
}
}