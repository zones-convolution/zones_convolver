#include "FFT.h"

namespace zones
{
void ForwardFFTUnordered (std::complex<float> * data, std::size_t num_points)
{
    for (auto depth = static_cast<int> (std::log2 (num_points)); depth > 0; --depth)
    {
        auto num_elements_in_group = static_cast<int> (std::pow (2, depth));
        auto num_groups = num_points / num_elements_in_group;
        auto half_num_elements_in_group = num_elements_in_group / 2;
        auto w_n = std::exp (std::complex<float> (
            0.0f, -juce::MathConstants<float>::twoPi / static_cast<float> (num_elements_in_group)));

        for (auto group_index = 0; group_index < num_groups; ++group_index)
        {
            auto group = &data [group_index * num_elements_in_group];

            std::complex<float> w {1.0f, 0.0f};
            for (auto element_index = 0; element_index < half_num_elements_in_group;
                 ++element_index)
            {
                auto mirrored_index = element_index + half_num_elements_in_group;
                auto point = group [element_index];

                group [element_index] += group [mirrored_index];
                group [mirrored_index] = (point - group [mirrored_index]) * w;
                w *= w_n;
            }
        }
    }
}

void InverseFFTUnordered (std::complex<float> * data, std::size_t num_points)
{
    for (auto depth = 1; depth <= static_cast<int> (std::log2 (num_points)); ++depth)
    {
        auto num_elements_in_group = static_cast<int> (std::pow (2, depth));
        auto num_groups = num_points / num_elements_in_group;
        auto half_num_elements_in_group = num_elements_in_group / 2;
        auto w_n = std::exp (std::complex<float> (
            0.0f, juce::MathConstants<float>::twoPi / static_cast<float> (num_elements_in_group)));

        for (auto group_index = 0; group_index < num_groups; ++group_index)
        {
            auto group = &data [group_index * num_elements_in_group];

            std::complex<float> w {1.0f, 0.0f};
            for (auto element_index = 0; element_index < half_num_elements_in_group;
                 ++element_index)
            {
                auto mirrored_index = element_index + half_num_elements_in_group;
                auto b_n = group [mirrored_index] * w;

                group [mirrored_index] = (group [element_index] - b_n) * 0.5f;
                group [element_index] = (group [element_index] + b_n) * 0.5f;
                w *= w_n;
            }
        }
    }
}
}