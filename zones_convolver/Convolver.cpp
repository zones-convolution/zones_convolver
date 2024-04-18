#include "Convolver.h"

zones::Convolver::Convolver (juce::dsp::AudioBlock<const float> ir_block,
                             const juce::dsp::ProcessSpec & process_spec,
                             const zones::Convolver::ConvolverSpec & convolver_spec)
    : process_spec_ (process_spec)
    , convolver_spec_ (convolver_spec)
{
    num_convolution_channels_ = ir_block.getNumChannels ();
    auto max_block_size = process_spec_.maximumBlockSize;
    auto sample_rate = process_spec_.sampleRate;

    routing_buffer_.setSize (num_convolution_channels_, max_block_size);

    time_distributed_nupc_ = std::make_unique<TimeDistributedNUPC> (
        ir_block,
        juce::dsp::ProcessSpec {
            .sampleRate = sample_rate,
            .maximumBlockSize = max_block_size,
            .numChannels = static_cast<uint> (num_convolution_channels_),
        });
}

void zones::Convolver::Process (const juce::dsp::ProcessContextReplacing<float> & replacing)
{
    auto output_block = replacing.getOutputBlock ();

    juce::dsp::AudioBlock<float> routing_block {routing_buffer_};
    routing_block.clear ();

    for (auto convolution_channel = 0; convolution_channel < num_convolution_channels_;
         ++convolution_channel)
    {
        auto input_routing_channel = convolver_spec_.input_routing [convolution_channel];
        auto input_routing_block = output_block.getSingleChannelBlock (input_routing_channel);
        routing_block.getSingleChannelBlock (convolution_channel).copyFrom (input_routing_block);
    }

    time_distributed_nupc_->Process (
        {routing_block}); // THIS NEEDS TO HANDLE SMALLER AND LARGER BLOCKS...

    output_block.clear ();
    for (auto convolution_channel = 0; convolution_channel < num_convolution_channels_;
         ++convolution_channel)
    {
        auto output_routing_channel = convolver_spec_.output_routing [convolution_channel];
        auto output_routing_block = output_block.getSingleChannelBlock (output_routing_channel);
        output_routing_block.add (routing_block.getSingleChannelBlock (convolution_channel));
    }
}

void zones::Convolver::Reset ()
{
    time_distributed_nupc_->Reset ();
    routing_buffer_.clear ();
}
