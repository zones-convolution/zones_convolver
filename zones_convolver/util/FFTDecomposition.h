#pragma once

#include <juce_dsp/juce_dsp.h>

namespace FFTDecomposition
{
void ForwardDecompositionRadix2 (std::complex<float> * data,
                                 std::size_t num_points,
                                 std::size_t num_steps,
                                 std::size_t current_step);
void InverseDecompositionRadix2 ();
}
