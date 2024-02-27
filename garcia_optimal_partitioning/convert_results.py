import json


def normalise_result(dictionary, divisor):
    result_dict = {}
    for key, value in dictionary.items():
        updated_key = int(key) // divisor
        result_dict[updated_key] = value
    return result_dict


if __name__ == '__main__':
    latencies = [16, 32, 64, 128, 256, 512, 1024, 2048, 4096]

    header_prefix = """#pragma once

#include "util/Partitioning.h"

"""

    header_file = open("PartitionScheme.h", "w")
    header_file.write(header_prefix)

    impl_prefix = """#include "PartitionSchemes.h"

"""

    impl_file = open("PartitionScheme.cpp", "w")
    impl_file.write(impl_prefix)

    for latency in latencies:

        header_file.write("extern const PartitioningResults kGarciaResults{}; \n".format(latency))
        results = json.load(open("garcia_results_{}.json".format(latency), "r"))

        impl_file.write(f"const std::vector<PartitionScheme> kGarciaResults{latency} {{")
        for key in results:
            impl_file.write(
                f"""    
    PartitionScheme {{
        .ir_size_samples = {key},
        .layout = {{
""")
            result = normalise_result(results[key], latency)
            for partition_size, num_blocks in result.items():
                impl_file.write(
                    f"            PartitionLayout {{.partition_size_blocks = {partition_size}, .num_partitions = {num_blocks}}},\n")

            impl_file.write(
                """        }              
    },
""")

        impl_file.write("};\n")

    header_file.write("\nstatic const GarciaResults kGarciaResults { \n")
    for latency in latencies:
        header_file.write(f"    {{{latency}, &kGarciaResults{latency}}}, \n")
    header_file.write("}; \n")

    header_file.close()
    impl_file.close()
