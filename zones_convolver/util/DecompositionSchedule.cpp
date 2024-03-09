#include "DecompositionSchedule.h"

#include "FFTDecomposition.h"

StageBuffers::StageBuffers (int num_points)
{
    for (auto stage_index = 0; stage_index < kNumStages; ++stage_index)
        stages_.emplace_back (num_points, 1);

    for (auto & stage : stages_)
        stage.Clear ();
}

ComplexBuffer * StageBuffers::GetStage (StageBuffer stage)
{
    // Refactor as cyclic permutation table
    // 0, 1, 2,
    // 2, 0, 1,
    // 1, 2, 0
    //
    // headpos: 0, stage: A -> index 0 | stage: B -> index 1 | stage: C -> index 2
    // headpos: 1, stage: A -> index 2 | stage: B -> index 0 | stage: C -> index 1
    // headpos: 2, stage: A -> index 1 | stage: B -> index 2 | stage: C -> index 0

    return &stages_ [(stage - head_position_ + kNumStages) % kNumStages];
}

void StageBuffers::PromoteStages ()
{
    head_position_ = (head_position_ + 1) % kNumStages;
}

void StageBuffers::Clear ()
{
    for (auto & stage : stages_)
        stage.Clear ();
}

void DecompositionSchedule::ForwardDecompositionSchedule (int num_decompositions,
                                                          int fft_size,
                                                          int num_blocks,
                                                          std::complex<float> * data,
                                                          int phase)
{
    for (auto decomposition_level = 0; decomposition_level < num_decompositions;
         ++decomposition_level)
    {
        auto num_segments = static_cast<int> (std::pow (2, decomposition_level));
        auto num_steps = num_blocks / num_segments;
        if (phase >= num_blocks - num_steps)
        {
            auto relative_phase = phase % num_steps;
            FFTDecomposition::SegmentedForwardDecompositionRadix2 (
                data, fft_size, num_segments, num_steps, relative_phase);
        }
    }
}

void DecompositionSchedule::InverseDecompositionSchedule (int num_decompositions,
                                                          int fft_size,
                                                          int num_blocks,
                                                          std::complex<float> * data,
                                                          int phase)
{
    for (auto decomposition_level = num_decompositions - 1; decomposition_level >= 0;
         --decomposition_level)
    {
        auto num_segments = static_cast<int> (std::pow (2, decomposition_level));
        auto num_steps = num_blocks / num_segments;
        if (phase < num_steps)
            FFTDecomposition::SegmentedInverseDecompositionRadix2 (
                data, fft_size, num_segments, num_steps, phase);
    }
}