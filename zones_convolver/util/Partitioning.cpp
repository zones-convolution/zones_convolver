#include "Partitioning.h"

namespace zones
{
static PartitionScheme FindNearestPartitionScheme (const PartitioningResults * results,
                                                   int ir_num_samples)
{
    auto sorted_results = *results;
    std::sort (sorted_results.begin (),
               sorted_results.end (),
               [] (const PartitionScheme & a, const PartitionScheme & b)
               { return a.ir_size_samples < b.ir_size_samples; });

    for (auto scheme_index = 1; scheme_index < sorted_results.size (); ++scheme_index)
    {
        const auto & scheme = sorted_results.at (scheme_index);

        if (ir_num_samples < scheme.ir_size_samples)
        {
            auto upper_scheme_samples = sorted_results.at (scheme_index).ir_size_samples;
            auto lower_scheme_samples = sorted_results.at (scheme_index - 1).ir_size_samples;

            auto lower_dif = std::abs (ir_num_samples - lower_scheme_samples);
            auto higher_dif = std::abs (ir_num_samples - upper_scheme_samples);

            return sorted_results.at (higher_dif < lower_dif ? scheme_index : scheme_index - 1);
        }
    }

    return sorted_results.back ();
}

static bool IsLastComputedScheme (const PartitionScheme & scheme,
                                  const std::vector<PartitionScheme> & results)
{
    return results.back ().ir_size_samples == scheme.ir_size_samples;
}

static void ExtendSchemeToFitIR (PartitionScheme & scheme, int sample_difference, int block_size)
{
    auto & last_partition = scheme.layout.back ();
    auto last_partition_size_samples = block_size * last_partition.partition_size_blocks;

    auto num_partitions_to_add =
        static_cast<int> (std::ceil (sample_difference / last_partition_size_samples));
    last_partition.num_partitions += num_partitions_to_add;
}

static void ReduceSchemeToFitIR (PartitionScheme & scheme, int sample_difference, int block_size)
{
    auto remaining_samples_to_remove = std::abs (sample_difference);

    auto & last_partition = scheme.layout.back ();
    auto num_samples_in_last_partition = last_partition.GetSubConvolverSizeSamples (block_size);

    if (num_samples_in_last_partition > remaining_samples_to_remove)
    {
        auto last_partition_size_samples = block_size * last_partition.partition_size_blocks;
        auto num_partitions_to_remove = static_cast<int> (
            std::floor (remaining_samples_to_remove / last_partition_size_samples));
        last_partition.num_partitions -= num_partitions_to_remove;
    }
    else
    {
        scheme.layout.pop_back ();
        ReduceSchemeToFitIR (
            scheme, remaining_samples_to_remove - num_samples_in_last_partition, block_size);
    }
}

std::vector<PartitionLayout>
GetPartitionScheme (const GarciaResults & garcia_results, int block_size, int ir_num_samples)
{
    auto partitioning_results = garcia_results.at (block_size);
    auto nearest_scheme = FindNearestPartitionScheme (partitioning_results, ir_num_samples);

    auto sample_difference = ir_num_samples - nearest_scheme.ir_size_samples;

    if (sample_difference > 0)
        ExtendSchemeToFitIR (nearest_scheme, sample_difference, block_size);
    else
        ReduceSchemeToFitIR (nearest_scheme, sample_difference, block_size);

    return nearest_scheme.layout;
}
}