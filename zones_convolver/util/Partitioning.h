#pragma once

#include <unordered_map>
#include <vector>

struct PartitionLayout
{
    int partition_size_blocks;
    int num_partitions;

    [[nodiscard]] int GetSubConvolverSizeSamples (int block_size) const
    {
        return partition_size_blocks * num_partitions * block_size;
    }
};

struct PartitionScheme
{
    int ir_size_samples;
    std::vector<PartitionLayout> layout;
};

using PartitioningResults = std::vector<PartitionScheme>;
using GarciaResults = std::unordered_map<int, const PartitioningResults *>;

static int GetNumPartitionsRequiredForSegment (int partition_size, int segment_size)
{
    return static_cast<int> (
        std::ceil (static_cast<float> (segment_size) / static_cast<float> (partition_size)));
}

static std::vector<PartitionLayout>
GetPartitionScheme (const GarciaResults & garcia_results, int block_size, int ir_num_samples);