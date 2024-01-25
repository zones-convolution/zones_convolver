#include "LambdaPtrConverter.h"

#include <catch2/catch_test_macros.hpp>
#include <zones_convolver/util/DecompositionSchedule.h>

template <typename Lambda>
DecompositionSchedule::DecompositionFunction LambdaToDecompositionFunction (Lambda && pFun)
{
    return LambdaPtrConverter<
               Lambda,
               std::function<void (std::complex<float> *, std::size_t, std::size_t, std::size_t)>,
               void,
               DecompositionSchedule::DecompositionFunction> (pFun)
        .Ptr ();
}

SCENARIO ("executing decomposition tasks", "[DecompositionSchedule]")
{
    GIVEN ("a decomposition task")
    {
        static constexpr auto kNumPoints = 16;
        static constexpr auto kNumSegments = 4;

        StageBuffers stage_buffers {kNumPoints * kNumSegments};
        DecompositionSchedule::DecompositionTask task {
            .stage_index = 1,
            .segment_index = 0,
            .num_points = kNumPoints,
            .num_steps = 1,
            .current_step = 0,
        };

        std::complex<float> * executed_data;
        auto mock_decomposition_func = LambdaToDecompositionFunction (
            [&] (std::complex<float> * data,
                 std::size_t num_points,
                 std::size_t num_steps,
                 std::size_t current_step)
            {
                executed_data = data;
                REQUIRE (num_points == task.num_points);
                REQUIRE (num_steps == task.num_steps);
                REQUIRE (current_step == task.current_step);
            });

        for (auto segment_index = 0; segment_index < kNumSegments; ++segment_index)
        {
            WHEN ("the task is executed for segment " + std::to_string (segment_index))
            {
                task.segment_index = segment_index;
                DecompositionSchedule::ExecuteDecompositionTask (
                    mock_decomposition_func, task, stage_buffers);

                THEN ("the expected data is the segment of the correct stage buffer")
                {
                    auto expected_stage = stage_buffers.GetStage (task.stage_index);
                    REQUIRE (executed_data ==
                             &expected_stage->GetWritePointer (0) [segment_index * kNumPoints]);
                }
            }
        }
    }
}

SCENARIO ("can create forward decomposition plan", "[DecompositionSchedule]")
{
    WHEN ("a plan is created for a 2 phase schedule with 1 decomposition")
    {
        auto plan_is_empty = [] (DecompositionSchedule::DecompositionPlan & plan, int num_phases)
        {
            REQUIRE (plan.size () == num_phases);
            std::for_each (plan.begin (),
                           plan.end (),
                           [] (DecompositionSchedule::DecompositionTask & task)
                           { REQUIRE (task.empty); });
        };

        std::array<std::tuple<int, int, int>, 2> permutations {
            std::tuple<int, int, int> {8, 2, 1},
            std::tuple<int, int, int> {2, 2, 0},
        };

        for (auto & permutation : permutations)
        {
            auto plan = DecompositionSchedule::CreateForwardDecompositionPlan (
                std::get<0> (permutation), std::get<1> (permutation), std::get<2> (permutation));
            THEN ("the plan should not perform decompositions")
            {
                plan_is_empty (plan, std::get<1> (permutation));
            }
        }
    }
}