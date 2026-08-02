#include "FFT.h"

#include <juce_core/juce_core.h>

namespace zones
{
FFTUnordered::FFTUnordered (std::size_t num_points)
    : num_points_ (num_points)
{
    auto num_depths = static_cast<int> (std::log2 (num_points));

    forward_twiddle_factors_.resize (num_depths);
    inverse_twiddle_factors_.resize (num_depths);

    for (auto depth = num_depths; depth > 0; --depth)
    {
        auto num_elements_in_group = static_cast<int> (std::pow (2, depth));
        auto half_num_elements_in_group = num_elements_in_group / 2;

        auto & forward_twiddle_group = forward_twiddle_factors_ [depth - 1];
        forward_twiddle_group.resize (half_num_elements_in_group);
        auto & inverse_twiddle_group = inverse_twiddle_factors_ [depth - 1];
        inverse_twiddle_group.resize (half_num_elements_in_group);

        for (auto element_index = 0; element_index < half_num_elements_in_group; ++element_index)
        {
            forward_twiddle_group [element_index] = std::exp (std::complex<float> (
                0.0f,
                -juce::MathConstants<float>::twoPi * static_cast<float> (element_index) /
                    static_cast<float> (num_elements_in_group)));
            inverse_twiddle_group [element_index] = std::exp (std::complex<float> (
                0.0f,
                juce::MathConstants<float>::twoPi * static_cast<float> (element_index) /
                    static_cast<float> (num_elements_in_group)));
        }
    }
}

void FFTUnordered::Forward (std::complex<float> * data, std::size_t num_points)
{
    jassert (num_points == num_points_);

    for (auto depth = static_cast<int> (std::log2 (num_points)); depth > 0; --depth)
    {
        auto num_elements_in_group = static_cast<int> (std::pow (2, depth));
        auto num_groups = num_points / num_elements_in_group;
        auto half_num_elements_in_group = num_elements_in_group / 2;
        auto const & twiddle_group = forward_twiddle_factors_ [depth - 1];

        for (auto group_index = 0; group_index < num_groups; ++group_index)
        {
            auto group = &data [group_index * num_elements_in_group];

            for (auto element_index = 0; element_index < half_num_elements_in_group;
                 ++element_index)
            {
                auto mirrored_index = element_index + half_num_elements_in_group;
                auto point = group [element_index];

                group [element_index] += group [mirrored_index];
                group [mirrored_index] =
                    (point - group [mirrored_index]) * twiddle_group [element_index];
            }
        }
    }
}

void FFTUnordered::Inverse (std::complex<float> * data, std::size_t num_points)
{
    jassert (num_points == num_points_);

    for (auto depth = 1; depth <= static_cast<int> (std::log2 (num_points)); ++depth)
    {
        auto num_elements_in_group = static_cast<int> (std::pow (2, depth));
        auto num_groups = num_points / num_elements_in_group;
        auto half_num_elements_in_group = num_elements_in_group / 2;
        auto const & twiddle_group = inverse_twiddle_factors_ [depth - 1];

        for (auto group_index = 0; group_index < num_groups; ++group_index)
        {
            auto group = &data [group_index * num_elements_in_group];

            for (auto element_index = 0; element_index < half_num_elements_in_group;
                 ++element_index)
            {
                auto mirrored_index = element_index + half_num_elements_in_group;
                auto b_n = group [mirrored_index] * twiddle_group [element_index];

                group [mirrored_index] = (group [element_index] - b_n) * 0.5f;
                group [element_index] = (group [element_index] + b_n) * 0.5f;
            }
        }
    }
}
}