#pragma once
#include "ComplexBuffer.h"
#include "FFTDecomposition.h"

class StageBuffers
{
public:
    enum StageBuffer
    {
        kA = 0,
        kB = 1,
        kC = 2
    };

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
    bool empty = false;
    int stage_index;
    int segment_index;
    int num_points;
    int num_steps;
    int current_step;
};

using DecompositionFunction = void (*) (std::complex<float> * data,
                                        std::size_t num_points,
                                        std::size_t num_steps,
                                        std::size_t current_step);

using DecompositionPlan = std::vector<DecompositionTask>;

DecompositionPlan
CreateForwardDecompositionPlan (int partition_size, int num_phases, int num_decompositions);

DecompositionPlan
CreateInverseDecompositionPlan (int partition_size, int num_phases, int num_decompositions);

/**
 * NEEDS LOTS OF TESTING!!! haha... rip...
 */
inline void ExecuteForwardDecompositionPlan (const DecompositionPlan & plan,
                                             StageBuffers & stage_buffers,
                                             int num_points,
                                             int num_phases,
                                             int phase_number);

inline void ExecuteInverseDecompositionPlan (const DecompositionPlan & plan,
                                             StageBuffers & stage_buffers,
                                             int num_points,
                                             int num_phases,
                                             int phase_number);
}
