#include "Partitioning.h"

// std::vector<PartitionLayout>
// GetPartitionScheme (const GarciaResults & garcia_results, int block_size, int ir_num_samples)
//{
//     std::vector<PartitionLayout> partition_scheme;
//
//     static constexpr auto kMaxNumPartitionsInUPC = 8;
//     auto max_num_samples_in_upc = kMaxNumPartitionsInUPC * block_size;
//     auto num_samples_in_upc = std::min (ir_num_samples, max_num_samples_in_upc);
//     auto num_partitions_in_upc =
//         GetNumPartitionsRequiredForSegment (block_size, num_samples_in_upc);
//     partition_scheme.emplace_back (
//         PartitionLayout {.partition_size_blocks = 1, .num_partitions = num_partitions_in_upc});
//
//     auto remaining_samples_to_convolve = ir_num_samples - num_samples_in_upc;
//     if (remaining_samples_to_convolve > 0)
//     {
//         static constexpr auto kMaxNumPartitionsPrimary = 8;
//         const auto kPrimaryPartitionSizeBlocks = 4;
//         const auto kPrimaryPartitionSizeSamples = kPrimaryPartitionSizeBlocks * block_size;
//
//         auto num_samples_primary = std::min (
//             remaining_samples_to_convolve, kPrimaryPartitionSizeSamples *
//             kMaxNumPartitionsPrimary);
//
//         auto num_partitions_primary =
//             GetNumPartitionsRequiredForSegment (kPrimaryPartitionSizeSamples,
//             num_samples_primary);
//
//         partition_scheme.emplace_back (
//             PartitionLayout {.partition_size_blocks = kPrimaryPartitionSizeBlocks,
//                              .num_partitions = num_partitions_primary});
//
//         remaining_samples_to_convolve -= num_samples_primary;
//     }
//
//     if (remaining_samples_to_convolve > 0)
//     {
//         const auto kSecondaryPartitionSizeBlocks = 16;
//         const auto kSecondaryPartitionSizeSamples = kSecondaryPartitionSizeBlocks * block_size;
//         auto num_partitions_secondary = GetNumPartitionsRequiredForSegment (
//             kSecondaryPartitionSizeSamples, remaining_samples_to_convolve);
//         partition_scheme.emplace_back (
//             PartitionLayout {.partition_size_blocks = kSecondaryPartitionSizeBlocks,
//                              .num_partitions = num_partitions_secondary});
//     }
//
//     return partition_scheme;
// }

std::vector<PartitionLayout>
GetPartitionScheme (const GarciaResults & garcia_results, int block_size, int ir_num_samples)
{
}