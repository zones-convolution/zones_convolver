
#include "TestUtils.h"
#include "catch2/benchmark/catch_benchmark.hpp"

#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <melatonin_test_helpers/melatonin_test_helpers.h>
#include <numeric>
#include <vector>
#include <zones_convolver/zones_convolver.h>

using namespace zones;

std::vector<double> TimeConvolver (int num_ir_channels,
                                   int num_process_channels,
                                   int convolver_block_size,
                                   int ir_length_samples,
                                   int input_length,
                                   int sample_rate = 48000,
                                   int partition_size_blocks = 2,
                                   Convolver::ConvolverSpec convolver_spec = {})
{
    // create IR block
    juce::AudioBuffer<float> ir;
    ir.clear ();
    ir.setSize (num_ir_channels, ir_length_samples);
    for (auto channel_index = 0; channel_index < num_ir_channels; ++channel_index)
    {
        auto channel_bias = (static_cast<float> (static_cast<float> (channel_index) + 1.f) /
                             (static_cast<float> (num_ir_channels) + 1.f));
        for (auto i = 0; i < ir_length_samples; ++i)
            ir.setSample (
                channel_index,
                i,
                static_cast<float> (
                    ((static_cast<float> (2 * i) / static_cast<float> (ir_length_samples)) - 1.f) *
                    channel_bias));
    }
    juce::dsp::AudioBlock<float> ir_block {ir};

    // create input buffer
    juce::AudioBuffer<float> input;
    input.setSize (num_process_channels, input_length);
    input.clear ();
    input.setSample (0, 0, 1);

    juce::dsp::AudioBlock<float> input_block {input};

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

    // create processspec
    juce::dsp::ProcessSpec spec {.sampleRate = static_cast<double> (sample_rate),
                                 .maximumBlockSize =
                                     static_cast<juce::uint32> (convolver_block_size),
                                 .numChannels = static_cast<juce::uint32> (num_process_channels)};

    // prepare convolver
    Convolver convolver {ir_block, spec, convolver_spec};
    // UniformPartitionedConvolver convolver {spec, ir_block};

    for (auto channel_index = 0; channel_index < input_block.getNumChannels (); ++channel_index)
        input_block.setSample (channel_index, 0, 1.f);

    auto & subblock_vector = block_size_subblocks;

    using Clock = std::chrono::steady_clock;
    using Duration = std::chrono::duration<double, std::micro>; // microseconds

    std::vector<double> processTimesUs;
    processTimesUs.reserve (subblock_vector.size ());

    for (auto & subblock : subblock_vector)
    {
        auto start = Clock::now ();

        convolver.Process (subblock);

        auto end = Clock::now ();

        processTimesUs.push_back (Duration (end - start).count ());
    }

    CompareConvolverOutput (input_block, ir_block);
    return processTimesUs;
}

TEST_CASE ("Convolver Benchmarks")
{
    auto num_ir_channels = 1;
    auto block_size = 1024;
    auto ir_length = 48000 * 10;
    auto input_routing = std::vector<int> ({0, 1, 2, 3, 4, 5, 6});
    auto output_routing = std::vector<int> ({0, 1, 2, 3, 4, 5, 6});
    auto internal_block_size = block_size;
    auto input_length = ir_length * 2;
    auto sample_rate = 48000;
    std::cout << "Convolver block size: " << block_size << "\n";
    std::cout << "Convolver ir length: " << ir_length << "\n";
    double blockTimeUs = 1000000.0 * block_size / sample_rate;
    std::cout << "Convolver block time: " << blockTimeUs << "\n";

    Convolver::ConvolverSpec convolver_spec = {.input_routing = input_routing,
                                               .output_routing = output_routing,
                                               .fade_strategy = Convolver::FadeStrategy::kInOut,
                                               .internal_block_size = internal_block_size};
    auto processTimesUs = TimeConvolver (num_ir_channels,
                                         num_ir_channels,
                                         block_size,
                                         ir_length,
                                         input_length,
                                         sample_rate,
                                         2,
                                         convolver_spec);

    auto minTime = *std::min_element (processTimesUs.begin (), processTimesUs.end ());
    auto maxTime = *std::max_element (processTimesUs.begin (), processTimesUs.end ());
    auto avgTime = std::accumulate (processTimesUs.begin (), processTimesUs.end (), 0.0) /
                   processTimesUs.size ();

    std::cout << "Process() timings (us)\n";
    std::cout << "  Calls: " << processTimesUs.size () << "\n";
    std::cout << "  Min : " << minTime << "\n";
    std::cout << "  Avg : " << avgTime << "\n";
    std::cout << "  Max : " << maxTime << "\n";

    auto cpuUsage = (avgTime / blockTimeUs) * 100.0;
    auto maxCpuUsage = (maxTime / blockTimeUs) * 100.0;

    std::cout << "CPU usage: " << cpuUsage << "%\n";

    std::cout << "Worst case CPU usage: " << maxCpuUsage << "%\n";

    std::ofstream file ("process_times.csv");

    std::cout << "Working directory: " << std::filesystem::current_path () << "\n";

    if (! file.is_open ())
    {
        std::cerr << "Failed to open process_times.csv\n";
        return;
    }

    std::cout << "CSV opened successfully\n";

    file << "block,time_us\n";

    for (size_t i = 0; i < processTimesUs.size (); ++i)
    {
        file << i << "," << processTimesUs [i] << "\n";
    }

    file.close ();

    std::cout << "CSV saved\n";
}
