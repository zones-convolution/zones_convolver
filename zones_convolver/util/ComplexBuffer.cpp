#include "ComplexBuffer.h"

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

void ComplexBuffer::AddFrom (const ComplexBuffer & buffer)
{
    jassert (num_channels_ == buffer.num_channels_);
    jassert (num_points_ == buffer.num_points_);

    for (auto point_index = 0; point_index < channel_data_.size (); ++point_index)
        channel_data_ [point_index] += buffer.channel_data_ [point_index];
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
