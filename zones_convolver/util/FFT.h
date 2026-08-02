#pragma once

#include <complex>
#include <vector>

namespace zones
{
class FFTUnordered
{
public:
    FFTUnordered (std::size_t num_points);
    void Forward (std::complex<float> * data, std::size_t num_points);
    void Inverse (std::complex<float> * data, std::size_t num_points);

private:
    std::size_t num_points_;
    std::vector<std::vector<std::complex<float>>> forward_twiddle_factors_;
    std::vector<std::vector<std::complex<float>>> inverse_twiddle_factors_;
};
}