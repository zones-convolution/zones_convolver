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

SplitBlock CircularBuffer::GetNext (std::size_t delay, bool advance)
{
    juce::dsp::AudioBlock<float> block {buffer_};
    auto num_samples = buffer_.getNumSamples ();
    auto next_head_position = (head_position_ + delay) % num_samples;

    auto first_samples_to_take = num_samples - next_head_position;
    auto wrapped_samples_to_take = num_samples - first_samples_to_take;

    if (advance)
        head_position_ = next_head_position;

    return {first_samples_to_take > 0
                ? block.getSubBlock (next_head_position, first_samples_to_take)
                : juce::dsp::AudioBlock<float> (),
            wrapped_samples_to_take > 0 ? block.getSubBlock (0, wrapped_samples_to_take)
                                        : juce::dsp::AudioBlock<float> ()};
}
