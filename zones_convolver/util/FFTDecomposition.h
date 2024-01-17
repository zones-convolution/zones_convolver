#include <juce_dsp/juce_dsp.h>

namespace FFTDecomposition
{
inline void ForwardDecompositionRadix2 (std::complex<float> * data,
                                        std::size_t num_points,
                                        std::size_t num_steps,
                                        std::size_t current_step)
{
    jassert (num_points > 0 && juce::isPowerOfTwo (num_points));
    jassert (num_steps > 0 && num_steps <= num_points && juce::isPowerOfTwo (num_steps));
    jassert (current_step < num_steps);

    auto half_num_points = num_points / 2;
    for (auto point_index = 0u; point_index < half_num_points - 1u; ++point_index)
    {
        auto a_n = data [point_index] + data [half_num_points];
        auto b_n = data [point_index] - data [half_num_points];
    }
}

inline void InverseDecompositionRadix2 ()
{
}
}