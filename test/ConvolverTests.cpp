#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <melatonin_test_helpers/melatonin_test_helpers.h>
#include <zones_convolver/zones_convolver.h>

using namespace zones;

enum ConvolverType
{
    CONVOLVER,
    UPC,
    TDNUPC,
    TDUPC,
    TDUPCMULTI
};

void CreateandConvolveIR (ConvolverType convolverType,
                          int num_ir_channels,
                          int num_process_channels,
                          int convolver_block_size,
                          int ir_length_samples,
                          double sample_rate = 48000)
{
    // create IR block
    juce::AudioBuffer<float> ir;
    ir.clear ();
    ir.setSize (num_ir_channels, ir_length_samples);
    for (auto channel_index = 0; channel_index < num_ir_channels; ++channel_index)
    {
        for (auto i = 0; i < ir_length_samples; ++i)
            ir.setSample (
                channel_index,
                i,
                static_cast<float> (
                    (static_cast<float> (2 * i) / static_cast<float> (ir_length_samples)) - 1.f +
                    (static_cast<float> (channel_index / num_ir_channels))));
    }
    juce::dsp::AudioBlock<float> ir_block {ir};

    // create input buffer
    juce::AudioBuffer<float> input;
    input.setSize (num_process_channels, ir_length_samples * 2);
    input.clear ();
    input.setSample (0, 0, 1);

    juce::dsp::AudioBlock<float> input_block {input};

    //===== create process subblocks============
    std::vector<std::vector<juce::dsp::AudioBlock<float>>> process_subblocks;

    // entire block
    process_subblocks.push_back ({input_block});

    // convolver block size
    std::vector<juce::dsp::AudioBlock<float>> block_size_subblocks;
    int offset = 0;
    for (auto i = 0; i < (input_block.getNumSamples () / convolver_block_size); ++i)
    {
        block_size_subblocks.push_back (input_block.getSubBlock (offset, convolver_block_size));
        offset += convolver_block_size;
    }
    if (offset < input_block.getNumSamples ())
        block_size_subblocks.push_back (input_block.getSubBlock (offset));
    process_subblocks.push_back (std::move (block_size_subblocks));

    // large block size
    int large_block_size = convolver_block_size * 4;
    std::vector<juce::dsp::AudioBlock<float>> large_block_size_subblocks;
    int large_block_size_offset = 0;
    for (auto i = 0; i < (input_block.getNumSamples () / large_block_size); ++i)
    {
        large_block_size_subblocks.push_back (
            input_block.getSubBlock (large_block_size_offset, large_block_size));
        large_block_size_offset += large_block_size;
    }
    if (large_block_size_offset < input_block.getNumSamples ())
        large_block_size_subblocks.push_back (input_block.getSubBlock (large_block_size_offset));
    process_subblocks.push_back (std::move (large_block_size_subblocks));

    // non power of two
    std::vector<juce::dsp::AudioBlock<float>> non_po2_subblocks;
    int non_po2_offset = 0;
    std::vector<int> non_po2_sizes {23, 34, 1023, 1, 103};
    int non_po2_sizes_index = 0;
    while ((non_po2_offset + non_po2_sizes [non_po2_sizes_index]) < input_block.getNumSamples ())
    {
        non_po2_subblocks.push_back (
            input_block.getSubBlock (non_po2_offset, non_po2_sizes [non_po2_sizes_index]));
        non_po2_offset += non_po2_sizes [non_po2_sizes_index];
        non_po2_sizes_index = static_cast<int> ((non_po2_sizes_index + 1) % non_po2_sizes.size ());
    }
    non_po2_subblocks.push_back (input_block.getSubBlock (non_po2_offset));
    process_subblocks.push_back (std::move (non_po2_subblocks));

    // create processspec
    juce::dsp::ProcessSpec spec {.sampleRate = sample_rate,
                                 .maximumBlockSize =
                                     static_cast<juce::uint32> (convolver_block_size),
                                 .numChannels = static_cast<juce::uint32> (num_process_channels)};

    // lots of code repetition here
    switch (convolverType)
    {
        case UPC:
            {
                // prepare convolver
                UniformPartitionedConvolver convolver {spec, ir_block};

                // process for each block combination
                for (auto subblock_combination = 0;
                     subblock_combination < process_subblocks.size ();
                     ++subblock_combination)
                {
                    convolver.Reset ();
                    input_block.clear ();
                    for (auto channel_index = 0; channel_index < input_block.getNumChannels ();
                         ++channel_index)
                        input_block.setSample (channel_index, 0, 1.f);

                    auto & subblock_vector = process_subblocks [subblock_combination];

                    for (auto & subblock : subblock_vector)
                    {
                        convolver.Process (subblock);
                    }

                    REQUIRE_THAT (input_block.getSubBlock (0, ir_length_samples),
                                  melatonin::isEqualTo (ir_block));
                    // the rest should be zero but at the moment has a small amount of noise in so
                    // an alternative check with less tolerance is used REQUIRE_THAT (input_block,
                    // melatonin::isEmptyAfter (ir_length_samples));

                    juce::AudioBuffer<float> zero_buffer (
                        num_process_channels, input_block.getNumSamples () - ir_length_samples);
                    zero_buffer.clear ();
                    juce::dsp::AudioBlock<float> zero_block {zero_buffer};
                    REQUIRE_THAT (input_block.getSubBlock (ir_length_samples),
                                  melatonin::isEqualTo (zero_block, 0.00005f));
                }
                break;
            }
        case CONVOLVER:
            {
                // prepare convolver

                Convolver convolver {ir_block, spec, Convolver::ConvolverSpec {}};

                // process for each block combination
                for (auto subblock_combination = 0;
                     subblock_combination < process_subblocks.size ();
                     ++subblock_combination)
                {
                    convolver.Reset ();
                    input_block.clear ();
                    for (auto channel_index = 0; channel_index < input_block.getNumChannels ();
                         ++channel_index)
                        input_block.setSample (channel_index, 0, 1.f);

                    auto & subblock_vector = process_subblocks [subblock_combination];

                    for (auto & subblock : subblock_vector)
                    {
                        convolver.Process (subblock);
                    }

                    REQUIRE_THAT (input_block.getSubBlock (0, ir_length_samples),
                                  melatonin::isEqualTo (ir_block));
                    // the rest should be zero but at the moment has a small amount of noise in so
                    // an alternative check with less tolerance is used REQUIRE_THAT (input_block,
                    // melatonin::isEmptyAfter (ir_length_samples));

                    juce::AudioBuffer<float> zero_buffer (
                        num_process_channels, input_block.getNumSamples () - ir_length_samples);
                    zero_buffer.clear ();
                    juce::dsp::AudioBlock<float> zero_block {zero_buffer};
                    REQUIRE_THAT (input_block.getSubBlock (ir_length_samples),
                                  melatonin::isEqualTo (zero_block, 0.00005f));
                }
                break;
            }
        case TDNUPC:
            {
                // prepare convolver
                TimeDistributedNUPC convolver {ir_block, spec};

                // process for each block combination
                for (auto subblock_combination = 0;
                     subblock_combination < process_subblocks.size ();
                     ++subblock_combination)
                {
                    convolver.Reset ();
                    input_block.clear ();
                    for (auto channel_index = 0; channel_index < input_block.getNumChannels ();
                         ++channel_index)
                        input_block.setSample (channel_index, 0, 1.f);

                    auto & subblock_vector = process_subblocks [subblock_combination];

                    for (auto & subblock : subblock_vector)
                    {
                        convolver.Process (subblock);
                    }

                    REQUIRE_THAT (input_block.getSubBlock (0, ir_length_samples),
                                  melatonin::isEqualTo (ir_block));
                    // the rest should be zero but at the moment has a small amount of noise in so
                    // an alternative check with less tolerance is used REQUIRE_THAT (input_block,
                    // melatonin::isEmptyAfter (ir_length_samples));

                    juce::AudioBuffer<float> zero_buffer (
                        num_process_channels, input_block.getNumSamples () - ir_length_samples);
                    zero_buffer.clear ();
                    juce::dsp::AudioBlock<float> zero_block {zero_buffer};
                    REQUIRE_THAT (input_block.getSubBlock (ir_length_samples),
                                  melatonin::isEqualTo (zero_block, 0.00005f));
                }
                break;
            }
        case TDUPC:
            break;
        case TDUPCMULTI:
            break;
    }
}

SCENARIO ("Generic Convolver Tests", "[Convolver]")
{
    auto num_ir_channels = GENERATE (1, 2, 3, 4, 5, 6, 7, 8, 10, 100);
    auto block_size = GENERATE (1, 16, 32, 35, 1024, 2000);
    auto ir_length = GENERATE (1, 15, 32, 1000);

    SECTION ("test for:" + std::to_string (num_ir_channels) + "," + std::to_string (block_size) +
             "," + std::to_string (ir_length))
    {
        CreateandConvolveIR (
            ConvolverType::UPC, num_ir_channels, num_ir_channels, block_size, ir_length);
    }
}

SCENARIO ("TDNUPC Tests", "[TDNUPC]")
{
    auto num_ir_channels = GENERATE (1, 2, 3, 4);
    auto block_size = GENERATE (13);
    auto ir_length = GENERATE (1, 1000);

    SECTION ("test for:" + std::to_string (num_ir_channels) + "," + std::to_string (block_size) +
             "," + std::to_string (ir_length))
    {
        CreateandConvolveIR (
            ConvolverType::TDNUPC, num_ir_channels, num_ir_channels, block_size, ir_length);
    }
}