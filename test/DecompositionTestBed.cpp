#include "TestUtils.h"
#include "zones_convolver/util/DecompositionSchedule.h"
#include "zones_convolver/util/FFT.h"

#include <catch2/catch_test_macros.hpp>
#include <juce_dsp/juce_dsp.h>

static constexpr auto kBlockSize = 1024;
static constexpr auto kNumBlocks = 16;

static constexpr auto kInputSize = kBlockSize * kNumBlocks;
static constexpr auto kFFTSize = kInputSize * 2;

juce::AudioBuffer<float> CreateInputBuffer (int input_size)
{
    juce::AudioBuffer<float> input_buffer {1, input_size};
    auto input_block = juce::dsp::AudioBlock<float> {input_buffer};
    input_block.fill (0.67f); // Fill with random value
    return input_buffer;
}

void FillStageBuffersHalfWay (StageBuffers & stage_buffers, int input_size)
{
    auto stage_a = stage_buffers.GetStage (StageBuffers::StageBuffer::kA)->GetWritePointer (0);
    for (auto sample_index = 0; sample_index < input_size; ++sample_index)
        stage_a [0 + sample_index] = std::complex<float> {0.5f, 0.f};
}

void TestBedForwardDecompositionRadix2 (std::complex<float> * data,
                                        std::size_t num_points,
                                        std::size_t num_steps,
                                        std::size_t current_step)
{
    jassert (num_points > 0 && juce::isPowerOfTwo (num_points));
    jassert (num_steps > 0 && num_steps <= num_points && juce::isPowerOfTwo (num_steps));
    jassert (current_step < num_steps);

    auto half_num_points = num_points / 2;
    auto step_size = half_num_points / num_steps;
    auto start_index = current_step * step_size;
    auto w_n = std::exp (std::complex<float> (
        0.0f, -juce::MathConstants<float>::twoPi / static_cast<float> (num_points)));

    for (auto point_index = start_index; point_index < start_index + step_size; ++point_index)
    {
        jassert (half_num_points + point_index < num_points);

        auto point = data [point_index];

        data [point_index] += data [half_num_points + point_index];
        data [half_num_points + point_index] =
            (point - data [half_num_points + point_index]) *
            std::pow (w_n, std::complex<float> {static_cast<float> (point_index), 0.0f});
    }
}

static void HandleStageA (int kBlockSize,
                          const juce::AudioBuffer<float> & input_buffer,
                          StageBuffers & stage_buffers,
                          int phase)
{
    // Fill stage A from input buffer
    auto stage_index = phase * kBlockSize;
    auto stage_a = stage_buffers.GetStage (StageBuffers::kA)->GetWritePointer (0);
    for (auto sample_index = 0; sample_index < kBlockSize; ++sample_index)
        stage_a [stage_index + sample_index] =
            std::complex<float> {input_buffer.getSample (0, sample_index), 0.f};

    // Level 0 decompositions happen in every phase
    TestBedForwardDecompositionRadix2 (stage_a, kFFTSize, kNumBlocks, phase);

    // Level 1 decompositions happen in the second half of phases
    if (phase >= kNumBlocks / 2)
    {
        auto relative_phase = phase - (kNumBlocks / 2);

        TestBedForwardDecompositionRadix2 (stage_a, kFFTSize / 2, kNumBlocks / 2, relative_phase);
        TestBedForwardDecompositionRadix2 (
            stage_a + (kFFTSize / 2), kFFTSize / 2, kNumBlocks / 2, relative_phase);
    }

    // Level 2 decompositions happen in the last quarter of phases
    if (phase >= 3 * (kNumBlocks / 4))
    {
        auto relative_phase = (phase - (kNumBlocks / 2)) / 2;

        TestBedForwardDecompositionRadix2 (stage_a, kFFTSize / 4, kNumBlocks / 4, relative_phase);
        TestBedForwardDecompositionRadix2 (
            stage_a + (kFFTSize / 4), kFFTSize / 4, kNumBlocks / 4, relative_phase);
        TestBedForwardDecompositionRadix2 (
            stage_a + (2 * kFFTSize / 4), kFFTSize / 4, kNumBlocks / 4, relative_phase);
        TestBedForwardDecompositionRadix2 (
            stage_a + (3 * kFFTSize / 4), kFFTSize / 4, kNumBlocks / 4, relative_phase);
    }
}

TEST_CASE ("decomposing a 16b partition")
{
    auto input_buffer = CreateInputBuffer (kInputSize);
    StageBuffers stage_buffers {kFFTSize};
    FillStageBuffersHalfWay (stage_buffers, kInputSize);

    for (auto stage = 0; stage < 3; ++stage)
    {
        stage_buffers.PromoteStages ();

        for (auto phase = 0; phase < kNumBlocks; ++phase)
        {
            switch (stage)
            {
                case 0:
                    HandleStageA (kBlockSize, input_buffer, stage_buffers, phase);
                    break;
                case 1:
                    {
                        if (phase < 8)
                        {
                            auto stage_b = stage_buffers.GetStage (StageBuffers::StageBuffer::kB);
                            auto sub_fft_size = kFFTSize / (static_cast<int> (std::pow (2, 3)));
                            auto sub_fft_data =
                                &stage_b->GetWritePointer (0) [phase * sub_fft_size];
                            ForwardFFTUnordered (sub_fft_data, sub_fft_size);
                        }
                    }
                    break;
                case 2:
                    {
                    }
                    break;
            }
        }
    }

    ComplexBuffer test_fft_data {kFFTSize, 1};
    test_fft_data.CopyFromAudioBlock (input_buffer);
    auto test_fft_data_ptr = test_fft_data.GetWritePointer (0);
    ForwardFFTUnordered (test_fft_data_ptr, kFFTSize);

    auto stage_c = stage_buffers.GetStage (StageBuffers::StageBuffer::kC);
    auto stage_c_ptr = stage_c->GetWritePointer (0);

    for (auto point_index = 0u; point_index < kFFTSize; ++point_index)
        REQUIRE (
            ApproximatelyEqualComplex (test_fft_data_ptr [point_index], stage_c_ptr [point_index]));
}
