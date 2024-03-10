#pragma once

namespace zones
{
void ForwardFFTUnordered (std::complex<float> * data, std::size_t num_points);
void InverseFFTUnordered (std::complex<float> * data, std::size_t num_points);
}