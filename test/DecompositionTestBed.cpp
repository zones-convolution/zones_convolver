#include "TestUtils.h"
#include "zones_convolver/util/DecompositionSchedule.h"
#include "zones_convolver/util/FFT.h"
#include "zones_convolver/util/TimeDomainConvolver.h"

#include <catch2/catch_test_macros.hpp>
#include <juce_dsp/juce_dsp.h>

static constexpr auto kBlockSize = 256;
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

// STAGE,
// PARTITION SIZE,
// DIVISIONS,
// NUM PHASES,
// PHASE
void TestBedForwardDecompositionRadix2 (std::complex<float> * data,
                                        std::size_t num_points,
                                        std::size_t num_segments,
                                        std::size_t num_steps,
                                        std::size_t current_step)
{
    jassert (num_segments > 0 && juce::isPowerOfTwo (num_segments));
    jassert (num_points > 0 && juce::isPowerOfTwo (num_points));
    jassert (num_steps > 0 && num_steps <= num_points && juce::isPowerOfTwo (num_steps));
    jassert (current_step < num_steps);

    auto segment_num_points = num_points / num_segments;
    auto half_num_points = segment_num_points / 2;
    auto step_size = half_num_points / num_steps;
    auto start_index = current_step * step_size;
    auto w_n = std::exp (std::complex<float> (
        0.0f, -juce::MathConstants<float>::twoPi / static_cast<float> (segment_num_points)));

    for (auto segment_index = 0; segment_index < num_segments; ++segment_index)
    {
        auto segment = &data [segment_index * segment_num_points];
        for (auto point_index = start_index; point_index < start_index + step_size; ++point_index)
        {
            jassert (half_num_points + point_index < segment_num_points);
            auto point = segment [point_index];

            segment [point_index] += segment [half_num_points + point_index];
            segment [half_num_points + point_index] =
                (point - segment [half_num_points + point_index]) *
                std::pow (w_n, std::complex<float> {static_cast<float> (point_index), 0.0f});
        }
    }
}

void TestBedInverseDecompositionRadix2 (std::complex<float> * data,
                                        std::size_t num_points,
                                        std::size_t num_segments,
                                        std::size_t num_steps,
                                        std::size_t current_step)
{
    jassert (num_segments > 0 && juce::isPowerOfTwo (num_segments));
    jassert (num_points > 0 && juce::isPowerOfTwo (num_points));
    jassert (num_steps > 0 && num_steps <= num_points && juce::isPowerOfTwo (num_steps));
    jassert (current_step < num_steps);

    auto segment_num_points = num_points / num_segments;
    auto half_num_points = segment_num_points / 2;
    auto step_size = half_num_points / num_steps;
    auto start_index = current_step * step_size;
    auto w_n = std::exp (std::complex<float> (
        0.0f, juce::MathConstants<float>::twoPi / static_cast<float> (segment_num_points)));

    for (auto segment_index = 0; segment_index < num_segments; ++segment_index)
    {
        auto segment = &data [segment_index * segment_num_points];
        for (auto point_index = start_index; point_index < start_index + step_size; ++point_index)
        {
            jassert (half_num_points + point_index < segment_num_points);
            auto b_n = segment [point_index + half_num_points] *
                       std::pow (w_n, std::complex<float> {static_cast<float> (point_index), 0.0f});

            segment [half_num_points + point_index] = (segment [point_index] - b_n) * 0.5f;
            segment [point_index] = (segment [point_index] + b_n) * 0.5f;
        }
    }
}

static void HandleStageA (int num_decompositions,
                          int block_size,
                          const juce::AudioBuffer<float> & input_buffer,
                          StageBuffers & stage_buffers,
                          int phase)
{
    // Fill stage A from input buffer
    auto stage_index = phase * block_size;
    auto stage_a = stage_buffers.GetStage (StageBuffers::kA)->GetWritePointer (0);
    for (auto sample_index = 0; sample_index < block_size; ++sample_index)
        stage_a [stage_index + sample_index] =
            std::complex<float> {input_buffer.getSample (0, stage_index + sample_index), 0.f};

    // Level 0 decompositions happen in every phase
    TestBedForwardDecompositionRadix2 (stage_a, kFFTSize, 1, kNumBlocks, phase);

    for (auto decomposition_level = 1; decomposition_level < num_decompositions;
         ++decomposition_level)
    {
        auto num_segments = static_cast<int> (std::pow (2, decomposition_level));
        auto num_steps = kNumBlocks / num_segments;
        if (phase >= kNumBlocks - num_steps)
        {
            auto relative_phase = phase % num_steps;
            TestBedForwardDecompositionRadix2 (
                stage_a, kFFTSize, num_segments, num_steps, relative_phase);
        }
    }
}

static void HandleStageC (int num_decompositions, StageBuffers & stage_buffers, int phase)
{
    auto stage_c = stage_buffers.GetStage (StageBuffers::kC)->GetWritePointer (0);

    for (auto decomposition_level = num_decompositions - 1; decomposition_level >= 1;
         --decomposition_level)
    {
        auto num_segments = static_cast<int> (std::pow (2, decomposition_level));
        auto num_steps = kNumBlocks / num_segments;
        if (phase < num_steps)
            TestBedInverseDecompositionRadix2 (stage_c, kFFTSize, num_segments, num_steps, phase);
    }

    // Level 0 decompositions happen in every phase
    TestBedInverseDecompositionRadix2 (stage_c, kFFTSize, 1, kNumBlocks, phase);
}

TEST_CASE ("decomposing a 16b partition")
{
    auto input_buffer = CreateInputBuffer (kInputSize);
    StageBuffers stage_buffers {kFFTSize};
    FillStageBuffersHalfWay (stage_buffers, kInputSize);

    auto num_decompositions = static_cast<int> (std::log2 (kNumBlocks / 2));

    for (auto stage = 0; stage < 3; ++stage)
    {
        stage_buffers.PromoteStages ();

        for (auto phase = 0; phase < kNumBlocks; ++phase)
        {
            switch (stage)
            {
                case 0:
                    HandleStageA (
                        num_decompositions, kBlockSize, input_buffer, stage_buffers, phase);
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
                    HandleStageC (num_decompositions, stage_buffers, phase);

                    ComplexBuffer test_fft_data {kFFTSize, 1};
                    test_fft_data.Clear ();

                    test_fft_data.CopyFromAudioBlock (input_buffer);
                    auto test_fft_data_ptr = test_fft_data.GetWritePointer (0);
                    ForwardFFTUnordered (test_fft_data_ptr, kFFTSize);
                    InverseFFTUnordered (test_fft_data_ptr, kFFTSize);

                    auto stage_c = stage_buffers.GetStage (StageBuffers::StageBuffer::kC);
                    auto stage_c_ptr = stage_c->GetWritePointer (0);

                    auto starting_index = phase * kBlockSize;
                    for (auto sample_index = starting_index;
                         sample_index < starting_index + kBlockSize;
                         ++sample_index)
                        REQUIRE (ApproximatelyEqualComplex (stage_c_ptr [sample_index],
                                                            test_fft_data_ptr [sample_index]));

                    if (phase == 15)
                    {
                        for (auto sample_index = 0; sample_index < kFFTSize; ++sample_index)
                            REQUIRE (ApproximatelyEqualComplex (stage_c_ptr [sample_index],
                                                                test_fft_data_ptr [sample_index]));
                    }
                    break;
            }
        }
    }
}

juce::AudioBuffer<float> CreateTestIrBuffer ()
{
    juce::AudioBuffer<float> ir_buffer {1, kInputSize};
    for (auto sample_index = 0; sample_index < kInputSize; ++sample_index)
        ir_buffer.setSample (0, sample_index, std::sin (static_cast<float> (sample_index) * 0.1f));

    return ir_buffer;
}

// TEST_CASE ("decomposing a 16b partition with conv")
//{
//     auto input_buffer = CreateInputBuffer (kInputSize);
//     auto ir_buffer = CreateTestIrBuffer ();
//
//     StageBuffers stage_buffers {kFFTSize};
//
//     ComplexBuffer transformed_ir {kFFTSize, 1};
//     transformed_ir.Clear ();
//     transformed_ir.CopyFromAudioBlock (ir_buffer);
//     ForwardFFTUnordered (transformed_ir.GetWritePointer (0), kFFTSize);
//     auto transformed_ir_ptr = transformed_ir.GetWritePointer (0);
//
//     TimeDomainConvolver time_domain_convolver;
//     juce::dsp::ProcessSpec spec {44100, kInputSize, 1};
//     time_domain_convolver.prepare (spec);
//     time_domain_convolver.LoadImpulseResponse (ir_buffer, 44100);
//
//     auto time_domain_convolution_buffer = CreateInputBuffer (kInputSize);
//     juce::dsp::AudioBlock<float> time_domain_convolution_block {time_domain_convolution_buffer};
//     juce::dsp::ProcessContextReplacing<float> context {time_domain_convolution_block};
//     time_domain_convolver.process (context);
//
//     auto num_decompositions = static_cast<int> (std::log2 (kNumBlocks / 2));
//
//     for (auto stage = 0; stage < 3; ++stage)
//     {
//         stage_buffers.PromoteStages ();
//
//         for (auto phase = 0; phase < kNumBlocks; ++phase)
//         {
//             switch (stage)
//             {
//                 case 0:
//                     HandleStageA (
//                         num_decompositions, kBlockSize, input_buffer, stage_buffers, phase);
//                     break;
//                 case 1:
//                     {
//                         auto stage_b = stage_buffers.GetStage (StageBuffers::StageBuffer::kB);
//                         auto stage_b_ptr = stage_b->GetWritePointer (0);
//                         auto sub_fft_size = kFFTSize / (static_cast<int> (std::pow (2, 3)));
//
//                         if (phase % 2 == 0)
//                         {
//                             auto sub_fft_data = &stage_b_ptr [(phase / 2) * sub_fft_size];
//                             ForwardFFTUnordered (sub_fft_data, sub_fft_size);
//                         }
//
//                         auto half_sub_fft_size = sub_fft_size / 2;
//                         auto offset = (phase * half_sub_fft_size);
//                         for (auto point_index = 0; point_index < half_sub_fft_size;
//                         ++point_index)
//                         {
//                             auto point = stage_b_ptr [offset + point_index];
//                             auto ir_point = transformed_ir_ptr [offset + point_index];
//
//                             stage_b_ptr [offset + point_index] = point * ir_point;
//                         }
//
//                         if (phase % 2 != 0)
//                         {
//                             auto sub_fft_data = &stage_b_ptr [((phase - 1) / 2) * sub_fft_size];
//                             InverseFFTUnordered (sub_fft_data, sub_fft_size);
//                         }
//                     }
//                     break;
//                 case 2:
//                     HandleStageC (
//                         num_decompositions, kBlockSize, input_buffer, stage_buffers, phase);
//
//                     auto stage_c = stage_buffers.GetStage (StageBuffers::StageBuffer::kC);
//                     auto stage_c_ptr = stage_c->GetWritePointer (0);
//                     auto starting_index = phase * kBlockSize;
//
//                     auto tolerance = juce::Tolerance<float> ().withRelative (1.f);
//
//                     // TEST CONV RESULT
//                     for (auto sample_index = starting_index;
//                          sample_index < starting_index + kBlockSize;
//                          ++sample_index)
//                     {
//                         auto stage_c_sample = stage_c_ptr [sample_index].real ();
//                         auto time_domain_convolver_sample =
//                             time_domain_convolution_block.getSample (0, sample_index);
//                         REQUIRE (juce::approximatelyEqual (
//                             stage_c_sample, time_domain_convolver_sample, tolerance));
//                     }
//                     break;
//             }
//         }
//     }
// }
