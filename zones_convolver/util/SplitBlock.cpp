#include "SplitBlock.h"

SplitBlock::SplitBlock (juce::dsp::AudioBlock<float> first_block,
                        juce::dsp::AudioBlock<float> wrapped_block)
    : first_block_ (first_block)
    , wrapped_block_ (wrapped_block)
{
    total_num_samples_ = first_block.getNumSamples () + wrapped_block.getNumSamples ();
}

void SplitBlock::CopyFrom (const juce::dsp::AudioBlock<const float> block)
{
    auto num_samples = block.getNumSamples ();
    auto first_samples_to_copy = std::min (num_samples, first_block_.getNumSamples ());
    auto remaining_samples_to_copy = num_samples - first_samples_to_copy;

    if (first_samples_to_copy > 0)
        first_block_.copyFrom (block.getSubBlock (0, first_samples_to_copy));
    if (remaining_samples_to_copy > 0)
        wrapped_block_.copyFrom (
            block.getSubBlock (first_samples_to_copy, remaining_samples_to_copy));
}

void SplitBlock::AddFrom (const juce::dsp::AudioBlock<const float> block)
{
    auto num_samples = block.getNumSamples ();
    auto first_samples_to_copy = std::min (num_samples, first_block_.getNumSamples ());
    auto remaining_samples_to_copy = num_samples - first_samples_to_copy;

    if (first_samples_to_copy > 0)
        first_block_.add (block.getSubBlock (0, first_samples_to_copy));
    if (remaining_samples_to_copy > 0)
        wrapped_block_.add (block.getSubBlock (first_samples_to_copy, remaining_samples_to_copy));
}

void SplitBlock::CopyTo (juce::dsp::AudioBlock<float> block)
{
    block.copyFrom (first_block_);

    auto first_num_samples = first_block_.getNumSamples ();
    if (block.getNumSamples () > first_num_samples && wrapped_block_.getNumSamples () > 0)
        block.getSubBlock (first_num_samples).copyFrom (wrapped_block_);
}

void SplitBlock::AddTo (juce::dsp::AudioBlock<float> block)
{
    block.add (first_block_);

    auto first_num_samples = first_block_.getNumSamples ();
    if (block.getNumSamples () > first_num_samples && wrapped_block_.getNumSamples () > 0)
        block.getSubBlock (first_num_samples).add (wrapped_block_);
}

void SplitBlock::Clear ()
{
    first_block_.clear ();
    wrapped_block_.clear ();
}

SplitBlock SplitBlock::GetSubBlock (std::size_t num_samples)
{
    jassert (num_samples <= total_num_samples_);

    auto first_samples_to_take = std::min (num_samples, first_block_.getNumSamples ());
    auto wrapped_samples_to_take = num_samples - first_samples_to_take;

    return {first_block_.getSubBlock (0, first_samples_to_take),
            wrapped_block_.getSubBlock (0, wrapped_samples_to_take)};
}