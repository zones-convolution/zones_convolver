#include "DecompositionSchedule.h"

#include "FFTDecomposition.h"

namespace zones
{
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

DecompositionSchedule::DecompositionSchedule (int num_decompositions,
                                              int fft_num_points,
                                              int num_phases)
    : num_decompositions_ (num_decompositions)
    , fft_num_points_ (fft_num_points)
    , num_phases_ (num_phases)
{
    auto num_depths = static_cast<int> (std::log2 (fft_num_points));

    forward_twiddle_factors_.resize (num_depths);
    inverse_twiddle_factors_.resize (num_depths);

    for (auto depth = num_depths; depth > 0; --depth)
    {
        auto num_elements_in_group = static_cast<int> (std::pow (2, depth));
        auto half_num_elements_in_group = num_elements_in_group / 2;

        auto & forward_twiddle_group = forward_twiddle_factors_ [depth - 1];
        forward_twiddle_group.resize (half_num_elements_in_group);
        auto & inverse_twiddle_group = inverse_twiddle_factors_ [depth - 1];
        inverse_twiddle_group.resize (half_num_elements_in_group);

        const double angle_step =
            2.0 * juce::MathConstants<double>::pi / static_cast<double> (num_elements_in_group);

        for (auto element_index = 0; element_index < half_num_elements_in_group; ++element_index)
        {
            const double angle = angle_step * static_cast<double> (element_index);

            forward_twiddle_group [element_index] = std::complex<float> (
                static_cast<float> (std::cos (angle)), static_cast<float> (-std::sin (angle)));

            inverse_twiddle_group [element_index] = std::complex<float> (
                static_cast<float> (std::cos (angle)), static_cast<float> (std::sin (angle)));
        }
    }
}

void DecompositionSchedule::Forward (std::complex<float> * data, int phase)
{
    for (auto decomposition_level = 0; decomposition_level < num_decompositions_;
         ++decomposition_level)
    {
        auto num_segments = static_cast<int> (std::pow (2, decomposition_level));
        auto num_steps = num_phases_ / num_segments;
        if (phase >= num_phases_ - num_steps)
        {
            auto relative_phase = phase % num_steps;
            fft_decomposition::SegmentedForwardDecompositionRadix2 (data,
                                                                    fft_num_points_,
                                                                    num_segments,
                                                                    num_steps,
                                                                    relative_phase,
                                                                    forward_twiddle_factors_);
        }
    }
}

void DecompositionSchedule::Inverse (std::complex<float> * data, int phase)
{
    for (auto decomposition_level = num_decompositions_ + -1; decomposition_level >= 0;
         --decomposition_level)
    {
        auto num_segments = static_cast<int> (std::pow (2, decomposition_level));
        auto num_steps = num_phases_ / num_segments;
        if (phase < num_steps)
            fft_decomposition::SegmentedInverseDecompositionRadix2 (
                data, fft_num_points_, num_segments, num_steps, phase, inverse_twiddle_factors_);
    }
}
}