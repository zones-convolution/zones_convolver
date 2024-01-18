#include "FFTDecomposition.h"

void FFTDecomposition::ForwardDecompositionRadix2 (std::complex<float> * data,
                                                   std::size_t num_points,
                                                   std::size_t num_steps,
                                                   std::size_t current_step)
{
    jassert (num_points > 0 && juce::isPowerOfTwo (num_points));
    jassert (num_steps > 0 && num_steps <= num_points && juce::isPowerOfTwo (num_steps));
    jassert (current_step < num_steps);

    auto half_num_points = num_points / 2;
    auto step_size = half_num_points / num_steps;
    auto start_index = current_step * step_size;
    auto w_n = std::exp (std::complex<float> (
        0.0f, -juce::MathConstants<float>::twoPi / static_cast<float> (num_points)));

    for (auto point_index = start_index; point_index < start_index + step_size; ++point_index)
    {
        jassert (half_num_points + point_index < num_points);

        auto point = data [point_index];
        data [point_index] += data [half_num_points + point_index];
        data [half_num_points + point_index] =
            (point - data [half_num_points + point_index]) *
            std::pow (w_n, std::complex<float> {static_cast<float> (point_index), 0.0f});
    }
}

void FFTDecomposition::InverseDecompositionRadix2 ()
{
}