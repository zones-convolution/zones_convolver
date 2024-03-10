#include "TestUtils.h"

#include <catch2/catch_test_macros.hpp>
#include <juce_dsp/juce_dsp.h>
#include <zones_convolver/util/ComplexBuffer.h>
#include <zones_convolver/util/FFT.h>
#include <zones_convolver/util/FFTDecomposition.h>

using namespace zones;

TEST_CASE ("changing step size does not affect segmented forward decomposition output",
           "[SegmentedForwardDecompositionRadix2]")
{
    for (auto num_points : {8u, 16u, 32u, 64u, 128u})
    {
        auto num_step_sizes = static_cast<std::size_t> (std::log2 (num_points));
        ComplexBuffer inputs {num_points, num_step_sizes};

        for (auto channel_index = 0u; channel_index < inputs.GetNumChannels (); ++channel_index)
        {
            auto channel_pointer = inputs.GetWritePointer (channel_index);
            for (auto point_index = 0u; point_index < num_points; ++point_index)
            {
                channel_pointer [point_index] = {static_cast<float> (point_index) + 1.f, 0.f};
            }
        }

        for (auto step_size_index = 0u; step_size_index < num_step_sizes; ++step_size_index)
        {
            auto step_size = static_cast<std::size_t> (std::pow (2, step_size_index));
            auto num_steps = num_points / (2 * step_size);
            for (auto step = 0u; step < num_steps; ++step)
                fft_decomposition::SegmentedForwardDecompositionRadix2 (
                    inputs.GetWritePointer (step_size_index), num_points, 1, num_steps, step);
        }

        for (auto step_size_index = 0u; step_size_index < num_step_sizes - 1; ++step_size_index)
            for (auto point_index = 0u; point_index < num_points; ++point_index)
                REQUIRE (ApproximatelyEqualComplex (
                    inputs.GetReadPointer (step_size_index) [point_index],
                    inputs.GetReadPointer (step_size_index + 1u) [point_index]));
    }
}

TEST_CASE ("completes segmented decomposed FFT", "[SegmentedForwardDecompositionRadix2]")
{
    for (auto num_points : {2u, 4u, 8u, 32u, 64u, 1024u})
    {
        ComplexBuffer input {num_points, 2};
        input.Clear ();

        auto decomposed_fft_channel = input.GetWritePointer (0);
        auto juce_fft_channel = input.GetWritePointer (1);

        for (auto point_index = 0u; point_index < num_points; ++point_index)
            decomposed_fft_channel [point_index] = {static_cast<float> (point_index) + 1.f, 0.f};

        auto fft_order = static_cast<int> (std::log2 (num_points));
        auto juce_fft = juce::dsp::FFT {fft_order};
        juce_fft.perform (decomposed_fft_channel, juce_fft_channel, false);

        auto num_decomposition_layers = static_cast<std::size_t> (std::log2 (num_points));
        for (auto decomposition_layer = 0u; decomposition_layer < num_decomposition_layers;
             ++decomposition_layer)
        {
            auto num_sub_decompositions =
                static_cast<std::size_t> (std::pow (2, decomposition_layer));
            fft_decomposition::SegmentedForwardDecompositionRadix2 (
                decomposed_fft_channel, num_points, num_sub_decompositions, 1, 0);
        }

        /**
         * JUCE FFT performs DIT, to ensure points within each output have the same order a bit
         * reverse swap is performed on the DIF FFT outputs.
         */
        BitReverseSwap (decomposed_fft_channel, num_points);

        for (auto point_index = 0u; point_index < num_points; ++point_index)
            REQUIRE (ApproximatelyEqualComplex (decomposed_fft_channel [point_index],
                                                juce_fft_channel [point_index]));
    }
}

TEST_CASE ("segmented decomposed FFT followed by unordered FFT",
           "[SegmentedForwardDecompositionRadix2]")
{
    for (auto num_points : {32u, 64u, 1024u})
    {
        ComplexBuffer input {num_points, 2};
        input.Clear ();

        auto decomposed_fft_channel = input.GetWritePointer (0);
        auto juce_fft_channel = input.GetWritePointer (1);

        for (auto point_index = 0u; point_index < num_points; ++point_index)
            decomposed_fft_channel [point_index] = {static_cast<float> (point_index) + 1.f, 0.f};

        auto fft_order = static_cast<int> (std::log2 (num_points));
        auto juce_fft = juce::dsp::FFT {fft_order};
        juce_fft.perform (decomposed_fft_channel, juce_fft_channel, false);

        auto num_decomposition_layers = 3;
        for (auto decomposition_layer = 0u; decomposition_layer < num_decomposition_layers;
             ++decomposition_layer)
        {
            auto num_sub_decompositions =
                static_cast<std::size_t> (std::pow (2, decomposition_layer));
            fft_decomposition::SegmentedForwardDecompositionRadix2 (
                decomposed_fft_channel, num_points, num_sub_decompositions, 1, 0);
        }

        auto num_sub_decompositions =
            static_cast<std::size_t> (std::pow (2, num_decomposition_layers));
        auto sub_fft_size = num_points / num_sub_decompositions;

        for (auto sub_decomposition_index = 0; sub_decomposition_index < num_sub_decompositions;
             ++sub_decomposition_index)
            ForwardFFTUnordered (&decomposed_fft_channel [sub_decomposition_index * sub_fft_size],
                                 sub_fft_size);

        /**
         * JUCE FFT performs DIT, to ensure points within each output have the same order a bit
         * reverse swap is performed on the DIF FFT outputs.
         */
        BitReverseSwap (decomposed_fft_channel, num_points);

        for (auto point_index = 0u; point_index < num_points; ++point_index)
            REQUIRE (ApproximatelyEqualComplex (decomposed_fft_channel [point_index],
                                                juce_fft_channel [point_index]));
    }
}

TEST_CASE ("changing step size does not affect segmented inverse decomposition output",
           "[SegmentedInverseDecompositionRadix2]")
{
    for (auto num_points : {8u, 16u, 32u, 64u, 128u})
    {
        auto num_step_sizes = static_cast<std::size_t> (std::log2 (num_points));
        ComplexBuffer inputs {num_points, num_step_sizes};

        for (auto channel_index = 0u; channel_index < inputs.GetNumChannels (); ++channel_index)
        {
            auto channel_pointer = inputs.GetWritePointer (channel_index);
            for (auto point_index = 0u; point_index < num_points; ++point_index)
            {
                channel_pointer [point_index] = {static_cast<float> (point_index) + 1.f, 0.f};
            }
        }

        for (auto step_size_index = 0u; step_size_index < num_step_sizes; ++step_size_index)
        {
            auto step_size = static_cast<std::size_t> (std::pow (2, step_size_index));
            auto num_steps = num_points / (2 * step_size);
            for (auto step = 0u; step < num_steps; ++step)
                fft_decomposition::SegmentedInverseDecompositionRadix2 (
                    inputs.GetWritePointer (step_size_index), num_points, 1, num_steps, step);
        }

        for (auto step_size_index = 0u; step_size_index < num_step_sizes - 1; ++step_size_index)
            for (auto point_index = 0u; point_index < num_points; ++point_index)
                REQUIRE (ApproximatelyEqualComplex (
                    inputs.GetReadPointer (step_size_index) [point_index],
                    inputs.GetReadPointer (step_size_index + 1u) [point_index]));
    }
}

TEST_CASE ("completes segmented inverse decomposed FFT", "[SegmentedInverseDecompositionRadix2]")
{
    for (auto num_points : {8u, 16u, 32u, 64u, 128u})
    {
        ComplexBuffer input {num_points, 2};
        input.Clear ();

        auto decomposed_fft_channel = input.GetWritePointer (0);
        auto juce_fft_channel = input.GetWritePointer (1);

        for (auto point_index = 0u; point_index < num_points; ++point_index)
            decomposed_fft_channel [point_index] = {static_cast<float> (point_index) + 1.f, 0.f};

        auto fft_order = static_cast<int> (std::log2 (num_points));
        auto juce_fft = juce::dsp::FFT {fft_order};
        juce_fft.perform (decomposed_fft_channel, juce_fft_channel, true);

        /**
         * BIT Reverse must be performed here to arrange the correctly ordered FFT points for the
         * inverse DIF decomposition.
         */
        BitReverseSwap (decomposed_fft_channel, num_points);

        auto num_decomposition_layers = static_cast<std::size_t> (std::log2 (num_points));
        for (auto decomposition_layer = num_decomposition_layers; decomposition_layer > 0u;
             --decomposition_layer)
        {
            auto num_sub_decompositions =
                static_cast<std::size_t> (std::pow (2, decomposition_layer - 1));
            fft_decomposition::SegmentedInverseDecompositionRadix2 (
                decomposed_fft_channel, num_points, num_sub_decompositions, 1, 0);
        }

        for (auto point_index = 0u; point_index < num_points; ++point_index)
            REQUIRE (ApproximatelyEqualComplex (decomposed_fft_channel [point_index],
                                                juce_fft_channel [point_index]));
    }
}

TEST_CASE ("completes segmented inverse followed by unordered iFFT",
           "[SegmentedInverseDecompositionRadix2]")
{
    for (auto num_points : {32u, 64u, 128u})
    {
        ComplexBuffer input {num_points, 2};
        input.Clear ();

        auto decomposed_fft_channel = input.GetWritePointer (0);
        auto juce_fft_channel = input.GetWritePointer (1);

        for (auto point_index = 0u; point_index < num_points; ++point_index)
            decomposed_fft_channel [point_index] = {static_cast<float> (point_index) + 1.f, 0.f};

        auto fft_order = static_cast<int> (std::log2 (num_points));
        auto juce_fft = juce::dsp::FFT {fft_order};
        juce_fft.perform (decomposed_fft_channel, juce_fft_channel, true);

        /**
         * BIT Reverse must be performed here to arrange the correctly ordered FFT points for the
         * inverse DIF decomposition.
         */
        BitReverseSwap (decomposed_fft_channel, num_points);

        auto num_decomposition_layers = 3;

        auto num_sub_decompositions =
            static_cast<std::size_t> (std::pow (2, num_decomposition_layers));
        auto sub_fft_size = num_points / num_sub_decompositions;

        for (auto sub_decomposition_index = 0; sub_decomposition_index < num_sub_decompositions;
             ++sub_decomposition_index)
            InverseFFTUnordered (&decomposed_fft_channel [sub_decomposition_index * sub_fft_size],
                                 sub_fft_size);

        for (auto decomposition_layer = num_decomposition_layers; decomposition_layer > 0u;
             --decomposition_layer)
        {
            auto num_sub_decompositions =
                static_cast<std::size_t> (std::pow (2, decomposition_layer - 1));
            fft_decomposition::SegmentedInverseDecompositionRadix2 (
                decomposed_fft_channel, num_points, num_sub_decompositions, 1, 0);
        }

        for (auto point_index = 0u; point_index < num_points; ++point_index)
            REQUIRE (ApproximatelyEqualComplex (decomposed_fft_channel [point_index],
                                                juce_fft_channel [point_index]));
    }
}