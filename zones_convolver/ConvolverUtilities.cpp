#include "ConvolverUtilities.h"

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

ComplexBuffer::ComplexBuffer (std::size_t num_points, std::size_t num_channels)
    : num_channels_ (num_channels)
    , num_points_ (num_points)
{
    channel_data_.resize (num_points * num_channels);
    for (auto channel_index = 0; channel_index < num_channels; ++channel_index)
        channel_pointers_.push_back (&channel_data_ [channel_index * num_points]);
}

std::size_t ComplexBuffer::GetNumChannels () const
{
    return num_channels_;
}

std::size_t ComplexBuffer::GetNumPoints () const
{
    return num_points_;
}

const std::complex<float> * ComplexBuffer::GetReadPointer (std::size_t channel_index) const
{
    jassert (juce::isPositiveAndBelow (channel_index, num_channels_));
    return channel_pointers_ [channel_index];
}

std::complex<float> * ComplexBuffer::GetWritePointer (std::size_t channel_index)
{
    jassert (juce::isPositiveAndBelow (channel_index, num_channels_));
    return channel_pointers_ [channel_index];
}

const std::complex<float> * const * ComplexBuffer::GetArrayOfReadPointer () const
{
    return channel_pointers_.data ();
}

std::complex<float> * const * ComplexBuffer::GetArrayOfWritePointer ()
{
    return channel_pointers_.data ();
}

void ComplexBuffer::Clear ()
{
    GetContinuousBlock ().clear ();
}

void ComplexBuffer::ComplexMultiplyFrom (const ComplexBuffer & a, const ComplexBuffer & b)
{
    jassert (num_channels_ == a.num_channels_ && a.num_channels_ == b.num_channels_);
    jassert (num_points_ == a.num_points_ && a.num_points_ == b.num_points_);

    for (auto point_index = 0; point_index < channel_data_.size (); ++point_index)
        channel_data_ [point_index] = a.channel_data_ [point_index] * b.channel_data_ [point_index];
}

void ComplexBuffer::ComplexMultiplyAccumulateFrom (const ComplexBuffer & a, const ComplexBuffer & b)
{
    jassert (num_channels_ == a.num_channels_ && a.num_channels_ == b.num_channels_);
    jassert (num_points_ == a.num_points_ && a.num_points_ == b.num_points_);

    for (auto point_index = 0; point_index < channel_data_.size (); ++point_index)
        channel_data_ [point_index] +=
            a.channel_data_ [point_index] * b.channel_data_ [point_index];
}

void ComplexBuffer::CopyFromAudioBlock (const juce::dsp::AudioBlock<const float> block)
{
    jassert (num_channels_ == block.getNumChannels ());

    auto fill_points = std::min (num_points_, block.getNumSamples ());

    for (auto channel_index = 0u; channel_index < num_channels_; ++channel_index)
        for (auto point_index = 0u; point_index < fill_points; ++point_index)
            GetWritePointer (channel_index) [point_index] = {
                block.getSample (channel_index, point_index), 0.f};
}

juce::dsp::AudioBlock<float> ComplexBuffer::GetContinuousBlock ()
{
    auto write_pointers = reinterpret_cast<float * const *> (GetArrayOfWritePointer ());
    return {write_pointers, num_channels_, 2 * num_points_};
}

juce::dsp::AudioBlock<const float> ComplexBuffer::GetContinuousBlock () const
{
    auto write_pointers = reinterpret_cast<const float * const *> (GetArrayOfReadPointer ());
    return {write_pointers, num_channels_, 2 * num_points_};
}

FrequencyDelayLine::FrequencyDelayLine (std::size_t num_channels,
                                        std::size_t num_blocks,
                                        std::size_t num_points_per_block)
{
    num_blocks_ = num_blocks;
    for (auto i = 0; i < num_blocks; ++i)
    {
        ComplexBuffer buffer {num_points_per_block, num_channels};
        buffer.Clear ();
        delay_line_.emplace_back (std::move (buffer));
    }
}

static inline std::size_t ReverseWrap (int a, int b)
{
    return static_cast<std::size_t> ((b + (a % b)) % b);
}

ComplexBuffer & FrequencyDelayLine::GetNextBlock ()
{
    head_position_ =
        ReverseWrap (static_cast<int> (head_position_) - 1, static_cast<int> (num_blocks_));
    return delay_line_ [head_position_];
}

const ComplexBuffer & FrequencyDelayLine::GetBlockWithOffset (std::size_t offset) const
{
    return delay_line_ [(head_position_ + offset) % num_blocks_];
}