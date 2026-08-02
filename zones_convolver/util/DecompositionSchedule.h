#pragma once
#include "ComplexBuffer.h"

namespace zones
{
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
    [[nodiscard]] ComplexBuffer * GetStage (StageBuffer stage);
    void PromoteStages ();
    void Clear ();

private:
    static constexpr auto kNumStages = 3;
    int head_position_ = 0;
    std::vector<ComplexBuffer> stages_;
};

class DecompositionSchedule
{
public:
    DecompositionSchedule (int num_decompositions, int fft_num_points, int num_phases);

    void Forward (std::complex<float> * data, int phase);
    void Inverse (std::complex<float> * data, int phase);

private:
    int num_decompositions_;
    int fft_num_points_;
    int num_phases_;

    std::vector<std::vector<std::complex<float>>> forward_twiddle_factors_;
    std::vector<std::vector<std::complex<float>>> inverse_twiddle_factors_;
};
}