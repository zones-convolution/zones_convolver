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

    auto start_index = current_step/num_steps * num_points;
    auto step_size = num_points / num_steps;

    auto half_num_points = num_points / 2;
    auto w_n = std::exp (std::complex<float> (0.0, -(M_PI * 2.0) / static_cast<float> (num_points)));


    for (auto point_index = start_index; point_index < start_index + step_size - 1u; ++point_index)
    {
        auto a_n = data [point_index] + data [half_num_points + point_index];
        auto b_n = (data [point_index] - data [half_num_points + point_index]) *  std::pow (w_n,
                                                                             std::complex<float> {static_cast<float> (point_index),
                                                                                                   0.0});
        data[point_index] = a_n;
        data[half_num_points + point_index] = b_n;
    }
}

inline void InverseDecompositionRadix2 ()
{
}
}

//auto half_num_points = num_points / 2;
//auto w_n = std::exp (std::complex<float> (0.0, -(M_PI * 2.0) / static_cast<float> (num_points)));
//
//
//for (auto point_index = 0u; point_index < half_num_points - 1u; ++point_index)
//{
//    auto a_n = data [point_index] + data [half_num_points + point_index];
//    auto b_n = (data [point_index] - data [half_num_points + point_index]) *  std::pow (w_n,
//                                                                                       std::complex<float> {static_cast<float> (point_index),
//                                                                                                            0.0});
//    data[point_index] = a_n;
//    data[half_num_points + point_index] = b_n;
//}