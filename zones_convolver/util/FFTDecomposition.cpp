#include "FFTDecomposition.h"

namespace zones
{
void fft_decomposition::SegmentedForwardDecompositionRadix2 (std::complex<float> * data,
                                                            std::size_t num_points,
                                                            std::size_t num_segments,
                                                            std::size_t num_steps,
                                                            std::size_t current_step)
{
    jassert (num_segments > 0 && juce::isPowerOfTwo (num_segments));
    jassert (num_points > 0 && juce::isPowerOfTwo (num_points));
    jassert (num_steps > 0 && num_steps <= num_points && juce::isPowerOfTwo (num_steps));
    jassert (current_step < num_steps);

    auto segment_num_points = num_points / num_segments;
    auto half_num_points = segment_num_points / 2;
    auto step_size = half_num_points / num_steps;
    auto start_index = current_step * step_size;
    auto w_n = std::exp (std::complex<float> (
        0.0f, -juce::MathConstants<float>::twoPi / static_cast<float> (segment_num_points)));

    for (auto segment_index = 0; segment_index < num_segments; ++segment_index)
    {
        auto segment = &data [segment_index * segment_num_points];
        for (auto point_index = start_index; point_index < start_index + step_size; ++point_index)
        {
            jassert (half_num_points + point_index < segment_num_points);
            auto point = segment [point_index];

            segment [point_index] += segment [half_num_points + point_index];
            segment [half_num_points + point_index] =
                (point - segment [half_num_points + point_index]) *
                std::pow (w_n, std::complex<float> {static_cast<float> (point_index), 0.0f});
        }
    }
}

void fft_decomposition::SegmentedInverseDecompositionRadix2 (std::complex<float> * data,
                                                            std::size_t num_points,
                                                            std::size_t num_segments,
                                                            std::size_t num_steps,
                                                            std::size_t current_step)
{
    jassert (num_segments > 0 && juce::isPowerOfTwo (num_segments));
    jassert (num_points > 0 && juce::isPowerOfTwo (num_points));
    jassert (num_steps > 0 && num_steps <= num_points && juce::isPowerOfTwo (num_steps));
    jassert (current_step < num_steps);

    auto segment_num_points = num_points / num_segments;
    auto half_num_points = segment_num_points / 2;
    auto step_size = half_num_points / num_steps;
    auto start_index = current_step * step_size;
    auto w_n = std::exp (std::complex<float> (
        0.0f, juce::MathConstants<float>::twoPi / static_cast<float> (segment_num_points)));

    for (auto segment_index = 0; segment_index < num_segments; ++segment_index)
    {
        auto segment = &data [segment_index * segment_num_points];
        for (auto point_index = start_index; point_index < start_index + step_size; ++point_index)
        {
            jassert (half_num_points + point_index < segment_num_points);
            auto b_n = segment [point_index + half_num_points] *
                       std::pow (w_n, std::complex<float> {static_cast<float> (point_index), 0.0f});

            segment [half_num_points + point_index] = (segment [point_index] - b_n) * 0.5f;
            segment [point_index] = (segment [point_index] + b_n) * 0.5f;
        }
    }
}
}