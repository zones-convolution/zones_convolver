#include <catch2/catch_test_macros.hpp>
#include <zones_convolver/util/FFTDecomposition.h>

SCENARIO ("can perform forward radix 2 decompositions", "[ForwardDecompositionRadix2]")
{
    GIVEN ("a complex array")
    {
        std::array<std::complex<float>, 8> input {
            std::complex<float> {1.f, 0.f},
            std::complex<float> {2.f, 0.f},
            std::complex<float> {3.f, 0.f},
            std::complex<float> {4.f, 0.f},
            std::complex<float> {5.f, 0.f},
            std::complex<float> {6.f, 0.f},
            std::complex<float> {7.f, 0.f},
            std::complex<float> {8.f, 0.f},
        };

        auto num_decomposition_layers = std::log2 (input.size ());

        for (auto decomposition_layer = 0; decomposition_layer < num_decomposition_layers;
             ++decomposition_layer)
        {
            auto num_sub_decompositions =
                static_cast<std::size_t> (std::pow (2, decomposition_layer));
            auto sub_decomposition_size = input.size () / num_sub_decompositions;

            for (auto sub_decomposition = 0u; sub_decomposition < num_sub_decompositions;
                 ++sub_decomposition)
            {
                auto num_steps = sub_decomposition_size;

                for (auto step = 0u; step < num_steps / 2; ++step)
                {
                    FFTDecomposition::ForwardDecompositionRadix2 (
                        &input [(sub_decomposition * sub_decomposition_size)],
                        sub_decomposition_size,
                        num_steps,
                        step);
                }
            }
        }

        for (auto point : input)
            std::cout << point << "\n";

        REQUIRE (false);
    }
}