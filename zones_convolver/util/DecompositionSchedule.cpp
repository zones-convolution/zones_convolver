#include "DecompositionSchedule.h"

StageBuffers::StageBuffers (int num_points)
{
    for (auto stage_index = 0; stage_index < kNumStages; ++stage_index)
        stages_.emplace_back (num_points, 1);
}

ComplexBuffer * StageBuffers::GetStage (int stage)
{
    return &stages_ [(head_position_ + stage) % kNumStages];
}

void StageBuffers::PromoteStages ()
{
    head_position_ = (head_position_ + 1) % kNumStages;
}

DecompositionSchedule::DecompositionPlan
DecompositionSchedule::CreateForwardDecompositionPlan (int partition_size,
                                                       int num_phases,
                                                       int num_decompositions)
{
    jassert (juce::isPowerOfTwo (partition_size) && juce::isPowerOfTwo (num_phases));

    DecompositionPlan plan;
    for (auto phase = 0; phase < num_phases; ++phase)
        plan.push_back ({.empty = true});

    auto half_num_phases = num_phases / 2;
    auto task_number = 0;

    for (auto decomposition_depth = 1; decomposition_depth < num_decompositions;
         ++decomposition_depth)
    {
        auto num_segments = static_cast<int> (std::pow (2, decomposition_depth));
        auto num_points = partition_size / num_segments;
        auto half_num_segments = num_segments / 2;

        for (auto segment_index = 0; segment_index < half_num_segments; ++segment_index)
        {
            plan [task_number + half_num_phases] = DecompositionTask {
                .empty = false,
                .stage_index = StageBuffers::StageBuffer::kA,
                .segment_index = segment_index,
                .num_points = num_points,
                .num_steps = 1,
                .current_step = 0,
            };

            plan [task_number] = DecompositionTask {
                .empty = false,
                .stage_index = StageBuffers::StageBuffer::kB,
                .segment_index = segment_index + half_num_segments,
                .num_points = num_points,
                .num_steps = 1,
                .current_step = 0,
            };

            task_number += 1;
        }
    }

    return plan;
}

DecompositionSchedule::DecompositionPlan
DecompositionSchedule::CreateInverseDecompositionPlan (int partition_size,
                                                       int num_phases,
                                                       int num_decompositions)
{
    jassert (juce::isPowerOfTwo (partition_size) && juce::isPowerOfTwo (num_phases));

    DecompositionPlan plan;
    for (auto phase = 0; phase < num_phases; ++phase)
        plan.push_back ({.empty = true});

    auto half_num_phases = num_phases / 2;
    auto quarter_num_phases = num_phases / 4;
    auto task_number = 0;

    for (auto decomposition_depth = num_decompositions; decomposition_depth > 0;
         --decomposition_depth)
    {
        auto num_segments = static_cast<int> (std::pow (2, decomposition_depth));
        auto num_points = partition_size / num_segments;
        auto half_num_segments = num_segments / 2;

        for (auto segment_index = 0; segment_index < half_num_segments; ++segment_index)
        {
            plan [task_number + half_num_phases + quarter_num_phases] = DecompositionTask {
                .empty = false,
                .stage_index = StageBuffers::StageBuffer::kB,
                .segment_index = segment_index,
                .num_points = num_points,
                .num_steps = 1,
                .current_step = 0,
            };

            plan [task_number + quarter_num_phases] = DecompositionTask {
                .empty = false,
                .stage_index = StageBuffers::StageBuffer::kC,
                .segment_index = segment_index + half_num_segments,
                .num_points = num_points,
                .num_steps = 1,
                .current_step = 0,
            };

            task_number += 1;
        }
    }

    return plan;
}

static inline void
ExecuteDecompositionTask (DecompositionSchedule::DecompositionFunction decomposition_function,
                          const DecompositionSchedule::DecompositionTask & decomposition_task,
                          StageBuffers & stage_buffers)
{
    auto stage = stage_buffers.GetStage (decomposition_task.stage_index);
    auto segment_offset = decomposition_task.segment_index * decomposition_task.num_points;
    decomposition_function (&stage->GetWritePointer (0) [segment_offset],
                            decomposition_task.num_points,
                            decomposition_task.num_steps,
                            decomposition_task.current_step);
}

inline void DecompositionSchedule::ExecuteForwardDecompositionPlan (const DecompositionPlan & plan,
                                                                    StageBuffers & stage_buffers,
                                                                    int num_points,
                                                                    int num_phases,
                                                                    int phase_number)
{
    ExecuteDecompositionTask (&FFTDecomposition::ForwardDecompositionRadix2,
                              DecompositionTask {.stage_index = StageBuffers::StageBuffer::kA,
                                                 .segment_index = 0,
                                                 .num_points = num_points,
                                                 .num_steps = num_phases,
                                                 .current_step = phase_number},
                              stage_buffers);

    auto & task = plan [phase_number];
    if (! task.empty)
        ExecuteDecompositionTask (
            &FFTDecomposition::ForwardDecompositionRadix2, task, stage_buffers);
}

inline void DecompositionSchedule::ExecuteInverseDecompositionPlan (
    const DecompositionSchedule::DecompositionPlan & plan,
    StageBuffers & stage_buffers,
    int num_points,
    int num_phases,
    int phase_number)
{
    auto & task = plan [phase_number];
    if (! task.empty)
        ExecuteDecompositionTask (
            &FFTDecomposition::InverseDecompositionRadix2, task, stage_buffers);

    ExecuteDecompositionTask (&FFTDecomposition::InverseDecompositionRadix2,
                              DecompositionTask {.stage_index = StageBuffers::StageBuffer::kC,
                                                 .segment_index = 0,
                                                 .num_points = num_points,
                                                 .num_steps = num_phases,
                                                 .current_step = phase_number},
                              stage_buffers);
}
