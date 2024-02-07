#include "TestUtils.h"
#include "zones_convolver/util/DecompositionSchedule.h"
#include "zones_convolver/util/FFT.h"

#include <catch2/catch_test_macros.hpp>
#include <juce_dsp/juce_dsp.h>

static constexpr auto kBlockSize = 4096;
static constexpr auto kNumBlocks = 16;

static constexpr auto kInputSize = kBlockSize * kNumBlocks;
static constexpr auto kFFTSize = kInputSize * 2;

juce::AudioBuffer<float> CreateInputBuffer (int input_size)
{
    juce::AudioBuffer<float> input_buffer {1, input_size};
    for (auto sample_index = 0; sample_index < kInputSize; ++sample_index)
        input_buffer.setSample (0, sample_index, static_cast<float> (sample_index));

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

void TestBedInverseDecompositionRadix2 (std::complex<float> * data,
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
        0.0f, juce::MathConstants<float>::twoPi / static_cast<float> (num_points)));

    for (auto point_index = start_index; point_index < start_index + step_size; ++point_index)
    {
        jassert (half_num_points + point_index < num_points);
        auto b_n = data [point_index + half_num_points] *
                   std::pow (w_n, std::complex<float> {static_cast<float> (point_index), 0.0f});

        data [half_num_points + point_index] = (data [point_index] - b_n) * 0.5f;
        data [point_index] = (data [point_index] + b_n) * 0.5f;
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
            std::complex<float> {input_buffer.getSample (0, stage_index + sample_index), 0.f};

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
        auto relative_phase = phase % (kNumBlocks / 4);

        TestBedForwardDecompositionRadix2 (stage_a, kFFTSize / 4, kNumBlocks / 4, relative_phase);
        TestBedForwardDecompositionRadix2 (
            stage_a + (kFFTSize / 4), kFFTSize / 4, kNumBlocks / 4, relative_phase);
        TestBedForwardDecompositionRadix2 (
            stage_a + (2 * kFFTSize / 4), kFFTSize / 4, kNumBlocks / 4, relative_phase);
        TestBedForwardDecompositionRadix2 (
            stage_a + (3 * kFFTSize / 4), kFFTSize / 4, kNumBlocks / 4, relative_phase);
    }
}

static void HandleStageC (int kBlockSize,
                          const juce::AudioBuffer<float> & input_buffer,
                          StageBuffers & stage_buffers,
                          int phase)
{
    auto stage_c = stage_buffers.GetStage (StageBuffers::kC)->GetWritePointer (0);

    // Level 2 decompositions happen in the last quarter of phases
    if (phase < (kNumBlocks / 4))
    {
        TestBedInverseDecompositionRadix2 (stage_c, kFFTSize / 4, kNumBlocks / 4, phase);
        TestBedInverseDecompositionRadix2 (
            stage_c + (kFFTSize / 4), kFFTSize / 4, kNumBlocks / 4, phase);
        TestBedInverseDecompositionRadix2 (
            stage_c + (2 * kFFTSize / 4), kFFTSize / 4, kNumBlocks / 4, phase);
        TestBedInverseDecompositionRadix2 (
            stage_c + (3 * kFFTSize / 4), kFFTSize / 4, kNumBlocks / 4, phase);
    }

    // Level 1 decompositions happen in the second half of phases
    if (phase < kNumBlocks / 2)
    {
        TestBedInverseDecompositionRadix2 (stage_c, kFFTSize / 2, kNumBlocks / 2, phase);
        TestBedInverseDecompositionRadix2 (
            stage_c + (kFFTSize / 2), kFFTSize / 2, kNumBlocks / 2, phase);
    }

    // Level 0 decompositions happen in every phase
    TestBedInverseDecompositionRadix2 (stage_c, kFFTSize, kNumBlocks, phase);
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
                        auto stage_b = stage_buffers.GetStage (StageBuffers::StageBuffer::kB);
                        auto sub_fft_size = kFFTSize / (static_cast<int> (std::pow (2, 3)));
                        auto relative_phase = phase % (kNumBlocks / 2);
                        auto sub_fft_data =
                            &stage_b->GetWritePointer (0) [relative_phase * sub_fft_size];

                        if (phase < 8)
                            ForwardFFTUnordered (sub_fft_data, sub_fft_size);
                        else
                            InverseFFTUnordered (sub_fft_data, sub_fft_size);

                        // When completed forward FFT'S check they match inputs
                        if (phase == 7)
                        {
                            ComplexBuffer test_fft_data {kFFTSize, 1};
                            test_fft_data.Clear ();

                            test_fft_data.CopyFromAudioBlock (input_buffer);
                            auto test_fft_data_ptr = test_fft_data.GetWritePointer (0);
                            ForwardFFTUnordered (test_fft_data_ptr, kFFTSize);

                            auto stage_b = stage_buffers.GetStage (StageBuffers::StageBuffer::kB);
                            auto stage_b_ptr = stage_b->GetWritePointer (0);

                            for (auto point_index = 0u; point_index < kFFTSize; ++point_index)
                                REQUIRE (ApproximatelyEqualComplex (test_fft_data_ptr [point_index],
                                                                    stage_b_ptr [point_index]));
                        }
                    }
                    break;
                case 2:
                    HandleStageC (kBlockSize, input_buffer, stage_buffers, phase);

                    if (phase == 15)
                    {
                        ComplexBuffer test_fft_data {kFFTSize, 1};
                        test_fft_data.Clear ();

                        test_fft_data.CopyFromAudioBlock (input_buffer);
                        auto test_fft_data_ptr = test_fft_data.GetWritePointer (0);
                        ForwardFFTUnordered (test_fft_data_ptr, kFFTSize);
                        InverseFFTUnordered (test_fft_data_ptr, kFFTSize);

                        auto stage_c = stage_buffers.GetStage (StageBuffers::StageBuffer::kC);
                        auto stage_c_ptr = stage_c->GetWritePointer (0);

                        for (auto sample_index = 0; sample_index < kFFTSize; ++sample_index)
                            REQUIRE (ApproximatelyEqualComplex (stage_c_ptr [sample_index],
                                                                test_fft_data_ptr [sample_index]));
                    }
                    break;
            }
        }
    }
}
