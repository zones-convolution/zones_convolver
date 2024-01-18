#include <catch2/catch_test_macros.hpp>
#include <zones_convolver/util/ComplexBuffer.h>
#include <zones_convolver/util/FFTDecomposition.h>

static bool ApproximatelyEqualComplex (const std::complex<float> & a, const std::complex<float> & b)
{
    return juce::approximatelyEqual (a.real (), b.real ()) &&
           juce::approximatelyEqual (a.imag (), b.imag ());
}

TEST_CASE ("changing step size does not affect decomposition output",
           "[ForwardDecompositionRadix2]")
{
    static constexpr auto kSubDecompositionSize = 32u;
    auto num_step_sizes = static_cast<std::size_t> (std::log2 (kSubDecompositionSize));

    ComplexBuffer inputs {kSubDecompositionSize, num_step_sizes};

    for (auto channel_index = 0; channel_index < inputs.GetNumChannels (); ++channel_index)
    {
        auto channel_pointer = inputs.GetWritePointer (channel_index);
        for (auto point_index = 0u; point_index < kSubDecompositionSize; ++point_index)
        {
            channel_pointer [point_index] = {static_cast<float> (point_index) + 1.f, 0.f};
        }
    }

    for (auto step_size_index = 0u; step_size_index < num_step_sizes; ++step_size_index)
    {
        auto step_size = static_cast<std::size_t> (std::pow (2, step_size_index));
        auto num_steps = kSubDecompositionSize / (2 * step_size);
        for (auto step = 0u; step < num_steps; ++step)
            FFTDecomposition::ForwardDecompositionRadix2 (
                inputs.GetWritePointer (step_size_index), kSubDecompositionSize, num_steps, step);
    }

    for (auto step_size_index = 0u; step_size_index < num_step_sizes - 1; ++step_size_index)
        for (auto point_index = 0u; point_index < kSubDecompositionSize; ++point_index)
            REQUIRE (ApproximatelyEqualComplex (
                inputs.GetReadPointer (step_size_index) [point_index],
                inputs.GetReadPointer (step_size_index + 1u) [point_index]));
}

//    GIVEN ("a complex array")
//    {
//        std::array<std::complex<float>, 8> input {
//            std::complex<float> {1.f, 0.f},
//            std::complex<float> {2.f, 0.f},
//            std::complex<float> {3.f, 0.f},
//            std::complex<float> {4.f, 0.f},
//            std::complex<float> {5.f, 0.f},
//            std::complex<float> {6.f, 0.f},
//            std::complex<float> {7.f, 0.f},
//            std::complex<float> {8.f, 0.f},
//        };
//
//        auto input_size = input.size ();
//        auto num_decomposition_layers = static_cast<std::size_t> (std::log2 (input_size));
//        auto step_size = 2u;
//
//        for (auto decomposition_layer = 0u; decomposition_layer <
//        num_decomposition_layers;
//             ++decomposition_layer)
//        {
//            auto num_sub_decompositions =
//                static_cast<std::size_t> (std::pow (2, decomposition_layer));
//            auto sub_decomposition_size = input_size / num_sub_decompositions;
//
//            for (auto sub_decomposition = 0u; sub_decomposition < num_sub_decompositions;
//                 ++sub_decomposition)
//            {
//                auto num_steps = sub_decomposition_size / step_size;
//                for (auto step = 0u; step < num_steps /2; ++step)
//                {
//                    FFTDecomposition::ForwardDecompositionRadix2 (
//                        &input [(sub_decomposition * sub_decomposition_size)],
//                        sub_decomposition_size,
//                        num_steps,
//                        step);
//                }
//            }
//        }
//
//        for (auto point : input)
//            std::cout << point << "\n";
//    }
