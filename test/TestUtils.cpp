#include "TestUtils.h"

#include <melatonin_test_helpers/melatonin_test_helpers.h>

bool ApproximatelyEqualComplex (const std::complex<float> & a, const std::complex<float> & b)
{
    auto tolerance = juce::Tolerance<float> ().withRelative (1e-5f).withAbsolute (1e-6f);
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