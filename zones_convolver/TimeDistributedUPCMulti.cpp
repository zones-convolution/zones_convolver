#include "TimeDistributedUPCMulti.h"

TimeDistributedUPCMulti::TimeDistributedUPCMulti (const juce::dsp::ProcessSpec & spec,
                                                  int partition_size_blocks,
                                                  juce::dsp::AudioBlock<const float> ir_segment)
{
    const auto num_channels = static_cast<int> (spec.numChannels);
    const auto ir_num_channels = static_cast<int> (ir_segment.getNumChannels ());

    for (auto channel_index = 0; channel_index < num_channels; ++channel_index)
        tdupcs_.emplace_back (TimeDistributedUPC (
            spec,
            partition_size_blocks,
            ir_segment.getSingleChannelBlock (channel_index % ir_num_channels)));
}

void TimeDistributedUPCMulti::Process (const juce::dsp::ProcessContextReplacing<float> & replacing)
{
    auto output_block = replacing.getOutputBlock ();
    auto num_channels = output_block.getNumChannels ();

    for (auto channel_index = 0; channel_index < num_channels; ++channel_index)
    {
        auto channel_block = output_block.getSingleChannelBlock (channel_index);
        juce::dsp::ProcessContextReplacing<float> channel_context {channel_block};
        tdupcs_ [channel_index].Process (channel_context);
    }
}

void TimeDistributedUPCMulti::Reset ()
{
    for (auto & tdupc : tdupcs_)
        tdupc.Reset ();
}