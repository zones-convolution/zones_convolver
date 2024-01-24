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

static inline void
ExecuteDecompositionTask (const DecompositionSchedule::DecompositionTask & decomposition_task,
                          StageBuffers & stage_buffers)
{
    auto stage = stage_buffers.GetStage (decomposition_task.stage_index);
    auto segment_offset = decomposition_task.segment_index * decomposition_task.num_points;
    FFTDecomposition::ForwardDecompositionRadix2 (&stage->GetWritePointer (0) [segment_offset],
                                                  decomposition_task.num_points,
                                                  decomposition_task.num_steps,
                                                  decomposition_task.current_step);
}

void DecompositionSchedule::ExecuteForwardDecompositionPlan (const DecompositionPlan & plan,
                                                             StageBuffers & stage_buffers,
                                                             int num_points,
                                                             int num_phases,
                                                             int phase_number)
{
    ExecuteDecompositionTask (DecompositionTask {.stage_index = 0,
                                                 .segment_index = 0,
                                                 .num_points = num_points,
                                                 .num_steps = num_phases,
                                                 .current_step = phase_number},
                              stage_buffers);

    auto task = plan [phase_number];
    if (task != nullptr)
        ExecuteDecompositionTask (*task, stage_buffers);
}