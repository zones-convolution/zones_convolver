#include "PartitionSchemes.h"

const std::vector<PartitionScheme> kGarciaResults1024 {
    PartitionScheme {
        .ir_size_samples = 2048,
        .layout = {PartitionLayout {.partition_size_blocks = 1024, .num_partitions = 2}}},
    PartitionScheme {
        .ir_size_samples = 4096,
        .layout = {PartitionLayout {.partition_size_blocks = 1024, .num_partitions = 4}}},
    PartitionScheme {
        .ir_size_samples = 131072,
        .layout = {PartitionLayout {.partition_size_blocks = 1024, .num_partitions = 16},
                   PartitionLayout {.partition_size_blocks = 8192, .num_partitions = 14}}}};