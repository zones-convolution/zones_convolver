#pragma once
#include "ComplexBuffer.h"
#include "FFTDecomposition.h"

class StageBuffers
{
public:
    explicit StageBuffers (int num_points);
    [[nodiscard]] ComplexBuffer * GetStage (int stage);
    void PromoteStages ();

private:
    static constexpr auto kNumStages = 3;
    int head_position_ = 0;
    std::vector<ComplexBuffer> stages_;
};

namespace DecompositionSchedule
{

struct DecompositionTask
{
    int stage_index;
    int segment_index;
    int num_points;
    int num_steps;
    int current_step;
};

using DecompositionPlan = std::vector<DecompositionTask *>;
void ExecuteForwardDecompositionPlan (const DecompositionPlan & plan,
                                      StageBuffers & stage_buffers,
                                      int num_points,
                                      int num_phases,
                                      int phase_number);

}
