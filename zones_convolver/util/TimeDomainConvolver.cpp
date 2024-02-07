#include "TimeDomainConvolver.h"

void TimeDomainConvolver::prepare (const juce::dsp::ProcessSpec & spec)
{
    delay_line_.prepare (spec);
}

void TimeDomainConvolver::process (const juce::dsp::ProcessContextReplacing<float> & replacing)
{
    if (ir_buffer_.getNumChannels () == 0)
        return;

    auto input_block = replacing.getInputBlock ();
    auto output_block = replacing.getOutputBlock ();

    for (auto channel_index = 0; channel_index < input_block.getNumChannels (); ++channel_index)
    {
        auto channel_block = output_block.getChannelPointer (channel_index);
        for (auto sample_index = 0; sample_index < input_block.getNumSamples (); ++sample_index)
        {
            delay_line_.pushSample (channel_index, channel_block [sample_index]);
            float convolved_output = 0.0f;

            auto ir_block = ir_buffer_.getReadPointer (channel_index);
            for (auto ir_sample_index = 0; ir_sample_index < ir_buffer_.getNumSamples ();
                 ++ir_sample_index)
            {
                convolved_output += ir_block [ir_sample_index] *
                                    delay_line_.popSample (channel_index, ir_sample_index, false);
            }

            channel_block [sample_index] = convolved_output;

            delay_line_.popSample (channel_index, -1, true);
        }
    }
}

void TimeDomainConvolver::reset ()
{
}

void TimeDomainConvolver::LoadImpulseResponse (juce::dsp::AudioBlock<float> ir_block,
                                               double sample_rate)
{
    auto num_samples = ir_block.getNumSamples ();
    ir_buffer_.setSize (ir_block.getNumChannels (), num_samples);
    juce::dsp::AudioBlock<float> {ir_buffer_}.copyFrom (ir_block);
    delay_line_.setMaximumDelayInSamples (num_samples);
    delay_line_.setDelay (num_samples);
}
