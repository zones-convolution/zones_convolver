#pragma once
#include "ComplexBuffer.h"

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
    void Clear ();

private:
    static constexpr auto kNumStages = 3;
    int head_position_ = 0;
    std::vector<ComplexBuffer> stages_;
};

namespace DecompositionSchedule
{
void ForwardDecompositionSchedule (int num_decompositions,
                                   int fft_size,
                                   int num_blocks,
                                   std::complex<float> * data,
                                   int phase);
void InverseDecompositionSchedule (int num_decompositions,
                                   int fft_size,
                                   int num_blocks,
                                   std::complex<float> * data,
                                   int phase);
}