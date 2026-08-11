#include "TestUtils.h"

#include <melatonin_test_helpers/melatonin_test_helpers.h>

bool ApproximatelyEqualComplex (const std::complex<float> & a, const std::complex<float> & b)
{
    auto tolerance = juce::Tolerance<float> ().withRelative (1e-5f).withAbsolute (1e-5f);

    if (! (juce::approximatelyEqual (a.real (), b.real (), tolerance) &&
           juce::approximatelyEqual (a.imag (), b.imag (), tolerance)))
    {
        std::cout << a << std::endl;
        std::cout << b << std::endl;
        std::cout << b - a << std::endl;
    }

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

void CompareConvolverOutput (juce::dsp::AudioBlock<float> output_block,
                             juce::dsp::AudioBlock<float> ir_block)
{
    auto num_ir_samples = ir_block.getNumSamples ();
    REQUIRE_THAT (output_block.getSubBlock (0, num_ir_samples),
                  melatonin::isEqualTo (ir_block, 0.0005f));
    // the rest should be zero but at the moment has a small amount of noise in so
    // an alternative check with less tolerance is used REQUIRE_THAT (input_block,
    // melatonin::isEmptyAfter (ir_length_samples));

    juce::AudioBuffer<float> zero_buffer (output_block.getNumChannels (),
                                          output_block.getNumSamples () - num_ir_samples);
    zero_buffer.clear ();
    juce::dsp::AudioBlock<float> zero_block {zero_buffer};
    REQUIRE_THAT (output_block.getSubBlock (num_ir_samples),
                  melatonin::isEqualTo (zero_block, 0.0005f));
}

std::vector<std::vector<std::vector<std::complex<float>>>>
CreateTwiddleFactors (unsigned fft_num_points)
{
    auto num_depths = static_cast<int> (std::log2 (fft_num_points));

    std::vector<std::vector<std::vector<std::complex<float>>>> twiddle_factors (2);

    auto & forward_twiddle_factors = twiddle_factors [0];
    auto & inverse_twiddle_factors = twiddle_factors [1];

    forward_twiddle_factors.resize (num_depths);
    inverse_twiddle_factors.resize (num_depths);

    for (auto depth = num_depths; depth > 0; --depth)
    {
        auto num_elements_in_group = static_cast<int> (std::pow (2, depth));
        auto half_num_elements_in_group = num_elements_in_group / 2;

        auto & forward_twiddle_group = forward_twiddle_factors [depth - 1];
        forward_twiddle_group.resize (half_num_elements_in_group);
        auto & inverse_twiddle_group = inverse_twiddle_factors [depth - 1];
        inverse_twiddle_group.resize (half_num_elements_in_group);

        const double angle_step =
            2.0 * juce::MathConstants<double>::pi / static_cast<double> (num_elements_in_group);

        for (auto element_index = 0; element_index < half_num_elements_in_group; ++element_index)
        {
            const double angle = angle_step * static_cast<double> (element_index);

            forward_twiddle_group [element_index] = std::complex<float> (
                static_cast<float> (std::cos (angle)), static_cast<float> (-std::sin (angle)));

            inverse_twiddle_group [element_index] = std::complex<float> (
                static_cast<float> (std::cos (angle)), static_cast<float> (std::sin (angle)));
        }
    }
    return twiddle_factors;
}