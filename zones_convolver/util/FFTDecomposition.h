#pragma once

#include <juce_dsp/juce_dsp.h>

namespace zones::fft_decomposition
{
void SegmentedForwardDecompositionRadix2 (std::complex<float> * data,
                                          std::size_t num_points,
                                          std::size_t num_segments,
                                          std::size_t num_steps,
                                          std::size_t current_step);
void SegmentedInverseDecompositionRadix2 (std::complex<float> * data,
                                          std::size_t num_points,
                                          std::size_t num_segments,
                                          std::size_t num_steps,
                                          std::size_t current_step);
}
