#pragma once

#include "util/Partitioning.h"

namespace zones
{
extern const PartitioningResults kGarciaResults16;
extern const PartitioningResults kGarciaResults32;
extern const PartitioningResults kGarciaResults64;
extern const PartitioningResults kGarciaResults128;
extern const PartitioningResults kGarciaResults256;
extern const PartitioningResults kGarciaResults512;
extern const PartitioningResults kGarciaResults1024;
extern const PartitioningResults kGarciaResults2048;
extern const PartitioningResults kGarciaResults4096;

static const GarciaResults kGarciaResults {
    {16, &kGarciaResults16},
    {32, &kGarciaResults32},
    {64, &kGarciaResults64},
    {128, &kGarciaResults128},
    {256, &kGarciaResults256},
    {512, &kGarciaResults512},
    {1024, &kGarciaResults1024},
    {2048, &kGarciaResults2048},
    {4096, &kGarciaResults4096},
};
}