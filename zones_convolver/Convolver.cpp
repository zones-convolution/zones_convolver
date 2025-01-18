#include "Convolver.h"

zones::Convolver::Convolver (juce::dsp::AudioBlock<const float> ir_block,
                             const juce::dsp::ProcessSpec & process_spec,
                             const zones::Convolver::ConvolverSpec & convolver_spec)
    : convolver_spec_ (convolver_spec)
{
    num_convolution_channels_ = ir_block.getNumChannels ();
    num_samples_ = ir_block.getNumSamples ();

    if (convolver_spec_.internal_block_size.has_value () &&
        convolver_spec_.internal_block_size != 0)
        max_block_size_ = std::min (static_cast<int> (process_spec.maximumBlockSize),
                                    *convolver_spec_.internal_block_size);
    else
        max_block_size_ = static_cast<int> (process_spec.maximumBlockSize);

    auto sample_rate = process_spec.sampleRate;

    routing_buffer_.setSize (num_convolution_channels_, max_block_size_);

    time_distributed_nupc_ = std::make_unique<TimeDistributedNUPC> (
        ir_block,
        juce::dsp::ProcessSpec {
            .sampleRate = sample_rate,
            .maximumBlockSize = static_cast<juce::uint32> (max_block_size_),
            .numChannels = static_cast<juce::uint32> (num_convolution_channels_),
        });
}

void zones::Convolver::Process (const juce::dsp::ProcessContextReplacing<float> & replacing)
{
    auto output_block = replacing.getOutputBlock ();
    auto block_size = static_cast<int> (output_block.getNumSamples ());

    juce::dsp::AudioBlock<float> routing_block {routing_buffer_};
    routing_block.clear ();

    auto num_samples_processed = 0;
    while (num_samples_processed < block_size)
    {
        auto num_samples_to_process =
            std::min (block_size - num_samples_processed, max_block_size_);

        auto output_sub_block =
            output_block.getSubBlock (num_samples_processed, num_samples_to_process);
        auto routing_sub_block = routing_block.getSubBlock (0, num_samples_to_process);

        for (auto convolution_channel = 0; convolution_channel < num_convolution_channels_;
             ++convolution_channel)
        {
            auto input_routing_channel = convolver_spec_.input_routing [convolution_channel];
            auto input_routing_block =
                output_sub_block.getSingleChannelBlock (input_routing_channel);
            routing_sub_block.getSingleChannelBlock (convolution_channel)
                .copyFrom (input_routing_block);
        }

        time_distributed_nupc_->Process ({routing_sub_block});

        output_sub_block.clear ();
        for (auto convolution_channel = 0; convolution_channel < num_convolution_channels_;
             ++convolution_channel)
        {
            auto output_routing_channel = convolver_spec_.output_routing [convolution_channel];
            auto output_routing_block =
                output_sub_block.getSingleChannelBlock (output_routing_channel);
            output_routing_block.add (
                routing_sub_block.getSingleChannelBlock (convolution_channel));
        }

        num_samples_processed += num_samples_to_process;
    }
}

void zones::Convolver::Reset ()
{
    time_distributed_nupc_->Reset ();
    routing_buffer_.clear ();
}

int zones::Convolver::GetNumSamples () const
{
    return num_samples_;
}

const zones::Convolver::ConvolverSpec & zones::Convolver::GetConvolverSpec () const
{
    return convolver_spec_;
}
