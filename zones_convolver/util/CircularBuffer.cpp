#include "CircularBuffer.h"

CircularBuffer::CircularBuffer (juce::AudioBuffer<float> & buffer)
    : buffer_ (buffer)
{
}

SplitBlock CircularBuffer::GetNext (std::size_t advancement)
{
    juce::dsp::AudioBlock<float> block {buffer_};
    auto num_samples = buffer_.getNumSamples ();
    head_position_ = (head_position_ + advancement) % num_samples;

    auto first_samples_to_take = num_samples - head_position_;
    auto wrapped_samples_to_take = num_samples - first_samples_to_take;

    return {block.getSubBlock (head_position_, first_samples_to_take),
            block.getSubBlock (0, wrapped_samples_to_take)};
}