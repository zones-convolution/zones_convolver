#include <complex>
#include <juce_dsp/juce_dsp.h>

bool ApproximatelyEqualComplex (const std::complex<float> & a, const std::complex<float> & b);
void BitReverseSwap (std::complex<float> * input, unsigned n);
void CompareConvolverOutput (juce::dsp::AudioBlock<float> output_block,
                             juce::dsp::AudioBlock<float> ir_block);
std::vector<std::vector<std::vector<std::complex<float>>>>
CreateTwiddleFactors (unsigned fft_num_points);