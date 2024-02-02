#pragma once

#include <juce_dsp/juce_dsp.h>

void ForwardFFTUnordered (std::complex<float> * data, std::size_t num_points);
void InverseFFTUnordered (std::complex<float> * data, std::size_t num_points);