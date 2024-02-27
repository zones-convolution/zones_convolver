#include "PartitionSchemes.h"

const std::vector<PartitionScheme> kGarciaResults16 {    
    PartitionScheme {
        .ir_size_samples = 32,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 2},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 64,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 4},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 128,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 8},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 256,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 16},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 512,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 8},
            PartitionLayout {.partition_size_blocks = 4, .num_partitions = 6},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 1024,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 8},
            PartitionLayout {.partition_size_blocks = 4, .num_partitions = 14},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 2048,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 16},
            PartitionLayout {.partition_size_blocks = 8, .num_partitions = 14},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 4096,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 8},
            PartitionLayout {.partition_size_blocks = 4, .num_partitions = 6},
            PartitionLayout {.partition_size_blocks = 16, .num_partitions = 14},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 8192,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 8},
            PartitionLayout {.partition_size_blocks = 4, .num_partitions = 14},
            PartitionLayout {.partition_size_blocks = 32, .num_partitions = 14},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 16384,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 16},
            PartitionLayout {.partition_size_blocks = 8, .num_partitions = 14},
            PartitionLayout {.partition_size_blocks = 64, .num_partitions = 14},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 32768,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 8},
            PartitionLayout {.partition_size_blocks = 4, .num_partitions = 6},
            PartitionLayout {.partition_size_blocks = 16, .num_partitions = 14},
            PartitionLayout {.partition_size_blocks = 128, .num_partitions = 14},
        }              
    },
};
const std::vector<PartitionScheme> kGarciaResults32 {    
    PartitionScheme {
        .ir_size_samples = 64,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 2},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 128,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 4},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 256,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 8},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 512,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 16},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 1024,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 8},
            PartitionLayout {.partition_size_blocks = 4, .num_partitions = 6},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 2048,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 8},
            PartitionLayout {.partition_size_blocks = 4, .num_partitions = 14},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 4096,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 16},
            PartitionLayout {.partition_size_blocks = 8, .num_partitions = 14},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 8192,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 8},
            PartitionLayout {.partition_size_blocks = 4, .num_partitions = 6},
            PartitionLayout {.partition_size_blocks = 16, .num_partitions = 14},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 16384,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 8},
            PartitionLayout {.partition_size_blocks = 4, .num_partitions = 14},
            PartitionLayout {.partition_size_blocks = 32, .num_partitions = 14},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 32768,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 16},
            PartitionLayout {.partition_size_blocks = 8, .num_partitions = 14},
            PartitionLayout {.partition_size_blocks = 64, .num_partitions = 14},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 65536,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 8},
            PartitionLayout {.partition_size_blocks = 4, .num_partitions = 6},
            PartitionLayout {.partition_size_blocks = 16, .num_partitions = 14},
            PartitionLayout {.partition_size_blocks = 128, .num_partitions = 14},
        }              
    },
};
const std::vector<PartitionScheme> kGarciaResults64 {    
    PartitionScheme {
        .ir_size_samples = 128,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 2},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 256,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 4},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 512,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 8},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 1024,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 16},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 2048,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 8},
            PartitionLayout {.partition_size_blocks = 4, .num_partitions = 6},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 4096,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 8},
            PartitionLayout {.partition_size_blocks = 4, .num_partitions = 14},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 8192,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 16},
            PartitionLayout {.partition_size_blocks = 8, .num_partitions = 14},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 16384,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 8},
            PartitionLayout {.partition_size_blocks = 4, .num_partitions = 6},
            PartitionLayout {.partition_size_blocks = 16, .num_partitions = 14},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 32768,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 8},
            PartitionLayout {.partition_size_blocks = 4, .num_partitions = 14},
            PartitionLayout {.partition_size_blocks = 32, .num_partitions = 14},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 65536,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 16},
            PartitionLayout {.partition_size_blocks = 8, .num_partitions = 14},
            PartitionLayout {.partition_size_blocks = 64, .num_partitions = 14},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 131072,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 16},
            PartitionLayout {.partition_size_blocks = 8, .num_partitions = 14},
            PartitionLayout {.partition_size_blocks = 64, .num_partitions = 30},
        }              
    },
};
const std::vector<PartitionScheme> kGarciaResults128 {    
    PartitionScheme {
        .ir_size_samples = 256,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 2},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 512,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 4},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 1024,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 8},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 2048,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 16},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 4096,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 8},
            PartitionLayout {.partition_size_blocks = 4, .num_partitions = 6},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 8192,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 8},
            PartitionLayout {.partition_size_blocks = 4, .num_partitions = 14},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 16384,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 16},
            PartitionLayout {.partition_size_blocks = 8, .num_partitions = 14},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 32768,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 16},
            PartitionLayout {.partition_size_blocks = 8, .num_partitions = 30},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 65536,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 8},
            PartitionLayout {.partition_size_blocks = 4, .num_partitions = 14},
            PartitionLayout {.partition_size_blocks = 32, .num_partitions = 14},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 131072,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 16},
            PartitionLayout {.partition_size_blocks = 8, .num_partitions = 14},
            PartitionLayout {.partition_size_blocks = 64, .num_partitions = 14},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 262144,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 16},
            PartitionLayout {.partition_size_blocks = 8, .num_partitions = 14},
            PartitionLayout {.partition_size_blocks = 64, .num_partitions = 30},
        }              
    },
};
const std::vector<PartitionScheme> kGarciaResults256 {    
    PartitionScheme {
        .ir_size_samples = 512,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 2},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 1024,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 4},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 2048,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 8},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 4096,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 16},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 8192,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 32},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 16384,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 8},
            PartitionLayout {.partition_size_blocks = 4, .num_partitions = 14},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 32768,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 16},
            PartitionLayout {.partition_size_blocks = 8, .num_partitions = 14},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 65536,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 16},
            PartitionLayout {.partition_size_blocks = 8, .num_partitions = 30},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 131072,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 8},
            PartitionLayout {.partition_size_blocks = 4, .num_partitions = 14},
            PartitionLayout {.partition_size_blocks = 32, .num_partitions = 14},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 262144,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 16},
            PartitionLayout {.partition_size_blocks = 8, .num_partitions = 14},
            PartitionLayout {.partition_size_blocks = 64, .num_partitions = 14},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 524288,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 16},
            PartitionLayout {.partition_size_blocks = 8, .num_partitions = 14},
            PartitionLayout {.partition_size_blocks = 64, .num_partitions = 30},
        }              
    },
};
const std::vector<PartitionScheme> kGarciaResults512 {    
    PartitionScheme {
        .ir_size_samples = 1024,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 2},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 2048,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 4},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 4096,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 8},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 8192,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 16},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 16384,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 32},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 32768,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 8},
            PartitionLayout {.partition_size_blocks = 4, .num_partitions = 14},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 65536,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 16},
            PartitionLayout {.partition_size_blocks = 8, .num_partitions = 14},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 131072,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 16},
            PartitionLayout {.partition_size_blocks = 8, .num_partitions = 30},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 262144,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 8},
            PartitionLayout {.partition_size_blocks = 4, .num_partitions = 14},
            PartitionLayout {.partition_size_blocks = 32, .num_partitions = 14},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 524288,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 16},
            PartitionLayout {.partition_size_blocks = 8, .num_partitions = 14},
            PartitionLayout {.partition_size_blocks = 64, .num_partitions = 14},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 1048576,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 16},
            PartitionLayout {.partition_size_blocks = 8, .num_partitions = 14},
            PartitionLayout {.partition_size_blocks = 64, .num_partitions = 30},
        }              
    },
};
const std::vector<PartitionScheme> kGarciaResults1024 {    
    PartitionScheme {
        .ir_size_samples = 2048,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 2},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 4096,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 4},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 8192,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 8},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 16384,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 16},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 32768,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 32},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 65536,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 8},
            PartitionLayout {.partition_size_blocks = 4, .num_partitions = 14},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 131072,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 16},
            PartitionLayout {.partition_size_blocks = 8, .num_partitions = 14},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 262144,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 16},
            PartitionLayout {.partition_size_blocks = 8, .num_partitions = 30},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 524288,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 8},
            PartitionLayout {.partition_size_blocks = 4, .num_partitions = 14},
            PartitionLayout {.partition_size_blocks = 32, .num_partitions = 14},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 1048576,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 16},
            PartitionLayout {.partition_size_blocks = 8, .num_partitions = 14},
            PartitionLayout {.partition_size_blocks = 64, .num_partitions = 14},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 2097152,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 16},
            PartitionLayout {.partition_size_blocks = 8, .num_partitions = 14},
            PartitionLayout {.partition_size_blocks = 64, .num_partitions = 30},
        }              
    },
};
const std::vector<PartitionScheme> kGarciaResults2048 {    
    PartitionScheme {
        .ir_size_samples = 4096,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 2},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 8192,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 4},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 16384,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 8},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 32768,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 16},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 65536,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 32},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 131072,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 8},
            PartitionLayout {.partition_size_blocks = 4, .num_partitions = 14},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 262144,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 16},
            PartitionLayout {.partition_size_blocks = 8, .num_partitions = 14},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 524288,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 16},
            PartitionLayout {.partition_size_blocks = 8, .num_partitions = 30},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 1048576,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 8},
            PartitionLayout {.partition_size_blocks = 4, .num_partitions = 14},
            PartitionLayout {.partition_size_blocks = 32, .num_partitions = 14},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 2097152,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 16},
            PartitionLayout {.partition_size_blocks = 8, .num_partitions = 14},
            PartitionLayout {.partition_size_blocks = 64, .num_partitions = 14},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 4194304,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 16},
            PartitionLayout {.partition_size_blocks = 8, .num_partitions = 14},
            PartitionLayout {.partition_size_blocks = 64, .num_partitions = 30},
        }              
    },
};
const std::vector<PartitionScheme> kGarciaResults4096 {    
    PartitionScheme {
        .ir_size_samples = 8192,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 2},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 16384,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 4},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 32768,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 8},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 65536,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 16},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 131072,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 32},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 262144,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 8},
            PartitionLayout {.partition_size_blocks = 4, .num_partitions = 14},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 524288,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 16},
            PartitionLayout {.partition_size_blocks = 8, .num_partitions = 14},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 1048576,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 16},
            PartitionLayout {.partition_size_blocks = 8, .num_partitions = 30},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 2097152,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 32},
            PartitionLayout {.partition_size_blocks = 16, .num_partitions = 30},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 4194304,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 16},
            PartitionLayout {.partition_size_blocks = 8, .num_partitions = 14},
            PartitionLayout {.partition_size_blocks = 64, .num_partitions = 14},
        }              
    },
    
    PartitionScheme {
        .ir_size_samples = 8388608,
        .layout = {
            PartitionLayout {.partition_size_blocks = 1, .num_partitions = 16},
            PartitionLayout {.partition_size_blocks = 8, .num_partitions = 14},
            PartitionLayout {.partition_size_blocks = 64, .num_partitions = 30},
        }              
    },
};
