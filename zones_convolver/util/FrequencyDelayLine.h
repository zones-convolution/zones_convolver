#pragma once

#include "ComplexBuffer.h"

#include <juce_dsp/juce_dsp.h>

class FrequencyDelayLine
{
public:
    FrequencyDelayLine (std::size_t num_channels,
                        std::size_t num_blocks,
                        std::size_t num_points_per_block);

    ComplexBuffer & GetNextBlock ();
    [[nodiscard]] ComplexBuffer & GetBlockWithOffset (std::size_t offset);

    void Clear ();

private:
    std::vector<ComplexBuffer> delay_line_;
    std::size_t head_position_ = 0;
    std::size_t num_blocks_;
};