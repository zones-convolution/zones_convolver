#include <catch2/catch_test_macros.hpp>
#include <melatonin_test_helpers/melatonin_test_helpers.h>
#include <zones_convolver/util/ComplexBuffer.h>

using namespace zones;

SCENARIO ("can construct complex buffers with different channel and point sizes", "[ComplexBuffer]")
{
    GIVEN ("a list of different configurations")
    {
        auto channel_configurations = {0, 1, 2, 3, 4, 8};
        auto point_configurations = {0, 1, 32, 256, 257};

        WHEN ("complex buffers are constructed")
        {
            for (std::size_t channel_size : channel_configurations)
            {
                for (std::size_t point_size : point_configurations)
                {
                    ComplexBuffer complex_buffer {point_size, channel_size};
                    THEN ("the dimensions are correct")
                    {
                        REQUIRE (complex_buffer.GetNumChannels () == channel_size);
                        REQUIRE (complex_buffer.GetNumPoints () == point_size);
                    }
                }
            }
        }
    }
}

SCENARIO ("can clear complex buffer", "[ComplexBuffer]")
{
    GIVEN ("a filled complex buffer")
    {
        auto num_channels = 2u;
        auto num_points = 32u;

        ComplexBuffer complex_buffer {num_points, num_channels};
        auto continuous_block = complex_buffer.GetContinuousBlock ();
        continuous_block.fill (1.0f);

        REQUIRE (continuous_block.getNumChannels () == num_channels);
        REQUIRE (continuous_block.getNumSamples () == 2 * num_points);
        REQUIRE (melatonin::blockIsFilled (continuous_block));

        WHEN ("the buffer is cleared")
        {
            complex_buffer.Clear ();
            THEN ("the buffer is filled with zeros")
            {
                REQUIRE (melatonin::blockIsEmpty (continuous_block));
                REQUIRE (complex_buffer.GetNumChannels () == num_channels);
                REQUIRE (complex_buffer.GetNumPoints () == num_points);
            }
        }
    }
}

SCENARIO ("can read and write into complex buffer", "[ComplexBuffer]")
{
    GIVEN ("a empty complex buffer")
    {
        auto num_channels = 8u;
        auto num_points = 32u;
        ComplexBuffer complex_buffer {num_points, num_channels};
        complex_buffer.Clear ();

        WHEN ("a sequence is written into the buffer")
        {
            for (auto channel_index = 0; channel_index < num_channels; ++channel_index)
            {
                auto channel = complex_buffer.GetWritePointer (channel_index);
                for (auto point_index = 0; point_index < num_points; ++point_index)
                {
                    channel [point_index] = {static_cast<float> (channel_index + point_index),
                                             static_cast<float> (channel_index * point_index)};
                }
            }

            THEN ("the sequence can be read from the buffer")
            {
                for (auto channel_index = 0; channel_index < num_channels; ++channel_index)
                {
                    auto channel = complex_buffer.GetReadPointer (channel_index);
                    for (auto point_index = 0; point_index < num_points; ++point_index)
                    {
                        REQUIRE (
                            channel [point_index] ==
                            std::complex<float> {static_cast<float> (channel_index + point_index),
                                                 static_cast<float> (channel_index * point_index)});
                    }
                }
            }
        }

        WHEN ("getting read and write pointers")
        {
            auto write_pointers = complex_buffer.GetArrayOfWritePointer ();
            auto read_pointers = complex_buffer.GetArrayOfReadPointer ();

            REQUIRE (write_pointers [0] + num_points == write_pointers [1]);
            REQUIRE (read_pointers [0] + num_points == read_pointers [1]);

            THEN ("the pointers match the call for single channels")
            {
                for (auto channel_index = 0u; channel_index < num_channels; ++channel_index)
                {
                    REQUIRE (complex_buffer.GetReadPointer (channel_index) ==
                             read_pointers [channel_index]);
                    REQUIRE (complex_buffer.GetWritePointer (channel_index) ==
                             write_pointers [channel_index]);
                }
            }
        }
    }
}

SCENARIO ("can access complex buffer as continuous audio block", "[ComplexBuffer]")
{
    GIVEN ("an empty complex buffer")
    {
        auto num_channels = 8u;
        auto num_points = 32u;
        ComplexBuffer complex_buffer {num_points, num_channels};
        complex_buffer.Clear ();

        WHEN ("getting a continuous block")
        {
            auto continuous_block = complex_buffer.GetContinuousBlock ();
            THEN ("the dimensions are correct")
            {
                REQUIRE (continuous_block.getNumChannels () == num_channels);
                REQUIRE (continuous_block.getNumSamples () == 2 * num_points);
            }
        }

        WHEN ("filling an obtained continuous block with a sequence")
        {
            auto continuous_block = complex_buffer.GetContinuousBlock ();
            REQUIRE (continuous_block.getNumChannels () == num_channels);
            REQUIRE (continuous_block.getNumSamples () == 2 * num_points);

            for (auto channel_index = 0u; channel_index < num_channels; ++channel_index)
                for (auto sample_index = 0u; sample_index < continuous_block.getNumSamples ();
                     ++sample_index)
                    continuous_block.setSample (
                        channel_index,
                        sample_index,
                        static_cast<float> ((2 * channel_index) + sample_index));

            THEN ("the complex sequence is correct")
            {
                for (auto channel_index = 0u; channel_index < num_channels; ++channel_index)
                {
                    auto channel = complex_buffer.GetReadPointer (channel_index);
                    for (auto point_index = 0u; point_index < num_points; ++point_index)
                    {
                        auto scaled_point_index = 2 * point_index;

                        REQUIRE (
                            channel [point_index] ==
                            std::complex<float> {
                                static_cast<float> (scaled_point_index + (2 * channel_index)),
                                static_cast<float> (scaled_point_index + (2 * channel_index) + 1)});

                        REQUIRE (juce::approximatelyEqual (
                            std::fmod (channel [point_index].real (), 2.f), 0.f)); // IS EVEN
                        REQUIRE (juce::approximatelyEqual (
                            std::fmod (channel [point_index].imag (), 2.f), 1.f)); // IS ODD
                    }
                }
            }
        }
    }
}

SCENARIO ("can perform complex multiplication", "[ComplexBuffer]")
{
    GIVEN ("an empty complex buffer")
    {
        auto num_channels = 9u;
        auto num_points = 27u;
        ComplexBuffer complex_buffer {num_points, num_channels};
        complex_buffer.Clear ();

        ComplexBuffer a {num_points, num_channels};
        ComplexBuffer b {num_points, num_channels};

        for (auto channel_index = 0; channel_index < num_channels; ++channel_index)
        {
            auto a_channel = a.GetWritePointer (channel_index);
            auto b_channel = b.GetWritePointer (channel_index);
            for (auto point_index = 0; point_index < num_points; ++point_index)
            {
                a_channel [point_index] =
                    std::complex<float> {static_cast<float> (channel_index + point_index),
                                         static_cast<float> (channel_index * point_index)};
                b_channel [point_index] =
                    std::complex<float> {static_cast<float> (channel_index + point_index + 1),
                                         static_cast<float> (channel_index * point_index + 1)};
            }
        }

        WHEN ("multiplying from")
        {
            complex_buffer.ComplexMultiplyFrom (a, b);

            THEN ("the complex multiplication is correct")
            {
                for (auto channel_index = 0; channel_index < num_channels; ++channel_index)
                {
                    auto channel = complex_buffer.GetReadPointer (channel_index);
                    auto a_channel = a.GetReadPointer (channel_index);
                    auto b_channel = b.GetReadPointer (channel_index);

                    for (auto point_index = 0; point_index < num_points; ++point_index)
                    {
                        REQUIRE (channel [point_index] ==
                                 a_channel [point_index] * b_channel [point_index]);
                    }
                }
            }
        }

        WHEN ("multiply accumulate from")
        {
            auto continuous_block = complex_buffer.GetContinuousBlock ();
            continuous_block.fill (1.0f);

            complex_buffer.ComplexMultiplyAccumulateFrom (a, b);

            THEN ("the complex multiply accumulate is correct")
            {
                for (auto channel_index = 0; channel_index < num_channels; ++channel_index)
                {
                    auto channel = complex_buffer.GetReadPointer (channel_index);
                    auto a_channel = a.GetReadPointer (channel_index);
                    auto b_channel = b.GetReadPointer (channel_index);

                    for (auto point_index = 0; point_index < num_points; ++point_index)
                    {
                        REQUIRE (channel [point_index] ==
                                 ((a_channel [point_index] * b_channel [point_index]) +
                                  std::complex<float> {1.f, 1.f}));
                    }
                }
            }
        }
    }
}

SCENARIO ("can copy from audio block")
{
    GIVEN ("an empty complex buffer")
    {
        auto num_channels = 12u;
        auto num_points = 15u;
        ComplexBuffer complex_buffer {num_points, num_channels};
        complex_buffer.Clear ();

        juce::AudioBuffer<float> buffer {static_cast<int> (num_channels),
                                         static_cast<int> (num_points)};
        juce::dsp::AudioBlock<float> block {buffer};

        for (auto channel_index = 0; channel_index < num_channels; ++channel_index)
        {
            auto channel = block.getChannelPointer (channel_index);
            for (auto sample_index = 0u; sample_index < num_points; ++sample_index)
            {
                channel [sample_index] = sample_index + channel_index;
            }
        }

        WHEN ("an audio block is copied")
        {
            complex_buffer.CopyFromAudioBlock (block);

            THEN ("the sequence is correctly copied")
            {
                for (auto channel_index = 0; channel_index < num_channels; ++channel_index)
                {
                    auto channel = complex_buffer.GetReadPointer (channel_index);
                    for (auto sample_index = 0u; sample_index < num_points; ++sample_index)
                    {
                        REQUIRE (channel [sample_index] ==
                                 std::complex<float> {
                                     static_cast<float> (sample_index + channel_index), 0.f});
                    }
                }
            }
        }
    }
}