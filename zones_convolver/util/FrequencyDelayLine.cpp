#include "FrequencyDelayLine.h"

FrequencyDelayLine::FrequencyDelayLine (std::size_t num_channels,
                                        std::size_t num_blocks,
                                        std::size_t num_points_per_block)
{
    num_blocks_ = num_blocks;
    for (auto i = 0; i < num_blocks; ++i)
    {
        ComplexBuffer buffer {num_points_per_block, num_channels};
        buffer.Clear ();
        delay_line_.emplace_back (std::move (buffer));
    }
}

static inline std::size_t ReverseWrap (int a, int b)
{
    return static_cast<std::size_t> ((b + (a % b)) % b);
}

ComplexBuffer & FrequencyDelayLine::GetNextBlock ()
{
    head_position_ =
        ReverseWrap (static_cast<int> (head_position_) - 1, static_cast<int> (num_blocks_));
    return delay_line_ [head_position_];
}

ComplexBuffer & FrequencyDelayLine::GetBlockWithOffset (std::size_t offset)
{
    return delay_line_ [(head_position_ + offset) % num_blocks_];
}

void FrequencyDelayLine::Clear ()
{
    for (auto & buffer : delay_line_)
        buffer.Clear ();
}
