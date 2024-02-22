#pragma once

static int GetNumPartitionsRequiredForSegment (int partition_size, int segment_size)
{
    return static_cast<int> (
        std::ceil (static_cast<float> (segment_size) / static_cast<float> (partition_size)));
}