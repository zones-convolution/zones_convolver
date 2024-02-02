#include "TestUtils.h"

#include <catch2/catch_test_macros.hpp>
#include <juce_dsp/juce_dsp.h>
#include <zones_convolver/util/ComplexBuffer.h>
#include <zones_convolver/util/FFT.h>

TEST_CASE ("completes forward FFT", "[ForwardFFTUnordered]")
{
    for (auto num_points : {2u, 4u, 8u, 32u, 64u, 1024u})
    {
        ComplexBuffer input {num_points, 2};
        input.Clear ();

        auto zones_fft_channel = input.GetWritePointer (0);
        auto juce_fft_channel = input.GetWritePointer (1);

        for (auto point_index = 0u; point_index < num_points; ++point_index)
            zones_fft_channel [point_index] = {static_cast<float> (point_index) + 1.f, 0.f};

        auto fft_order = static_cast<int> (std::log2 (num_points));
        auto juce_fft = juce::dsp::FFT {fft_order};
        juce_fft.perform (zones_fft_channel, juce_fft_channel, false);

        ForwardFFTUnordered (zones_fft_channel, num_points);

        /**
         * JUCE FFT performs DIT, to ensure points within each output have the same order a bit
         * reverse swap is performed on the DIF FFT outputs.
         */
        BitReverseSwap (zones_fft_channel, num_points);

        for (auto point_index = 0u; point_index < num_points; ++point_index)
            REQUIRE (ApproximatelyEqualComplex (zones_fft_channel [point_index],
                                                juce_fft_channel [point_index]));
    }
}

TEST_CASE ("completes inverse FFT", "[InverseFFTUnordered]")
{
    for (auto num_points : {2u, 4u, 8u, 32u, 64u, 1024u})
    {
        ComplexBuffer input {num_points, 2};
        input.Clear ();

        auto zones_fft_channel = input.GetWritePointer (0);
        auto juce_fft_channel = input.GetWritePointer (1);

        for (auto point_index = 0u; point_index < num_points; ++point_index)
            zones_fft_channel [point_index] = {static_cast<float> (point_index) + 1.f, 0.f};

        auto fft_order = static_cast<int> (std::log2 (num_points));
        auto juce_fft = juce::dsp::FFT {fft_order};
        juce_fft.perform (zones_fft_channel, juce_fft_channel, true);

        /**
         * BIT Reverse must be performed here to arrange the correctly ordered FFT points for the
         * inverse DIF.
         */
        BitReverseSwap (zones_fft_channel, num_points);

        InverseFFTUnordered (zones_fft_channel, num_points);

        for (auto point_index = 0u; point_index < num_points; ++point_index)
            REQUIRE (ApproximatelyEqualComplex (zones_fft_channel [point_index],
                                                juce_fft_channel [point_index]));
    }
}