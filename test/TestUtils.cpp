#include "TestUtils.h"

bool ApproximatelyEqualComplex (const std::complex<float> & a, const std::complex<float> & b)
{
    auto tolerance = juce::Tolerance<float> ().withRelative (0.01f);
    return juce::approximatelyEqual (a.real (), b.real (), tolerance) &&
           juce::approximatelyEqual (a.imag (), b.imag (), tolerance);
}

void BitReverseSwap (std::complex<float> * input, unsigned n)
{
    for (unsigned i = 0, j = 0; i < n; i++)
    {
        if (i < j)
        {
            auto tmp = input [i];
            input [i] = input [j];
            input [j] = tmp;
        }

        unsigned mask = i ^ (i + 1);
        unsigned rev = n / (mask + 1);
        j ^= n - rev;
    }
}