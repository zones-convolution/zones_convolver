#pragma once

#include <juce_dsp/juce_dsp.h>

namespace FFTDecomposition
{
void ForwardDecompositionRadix2 (std::complex<float> * data,
                                 std::size_t num_points,
                                 std::size_t num_steps,
                                 std::size_t current_step);
void InverseDecompositionRadix2 (std::complex<float> * data,
                                 std::size_t num_points,
                                 std::size_t num_steps,
                                 std::size_t current_step);

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
