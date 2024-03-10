#pragma once

/*
BEGIN_JUCE_MODULE_DECLARATION

 ID:               zones_convolver
 vendor:           Zones
 version:          1.0.0
 name:             Zones convolver
 description:      Convolution engine used by the Zones platform.
 license:          MIT
 dependencies:     juce_dsp

END_JUCE_MODULE_DECLARATION
*/

#include "ConvolutionEngine.h"
#include "PartitionSchemes.h"
#include "UniformPartitionedConvolver.h"
#include "time_distributed/TimeDistributedNUPC.h"
#include "time_distributed/TimeDistributedUPC.h"
#include "time_distributed/TimeDistributedUPCMulti.h"
#include "util/CircularBuffer.h"
#include "util/ComplexBuffer.h"
#include "util/DecompositionSchedule.h"
#include "util/FFT.h"
#include "util/FFTDecomposition.h"
#include "util/FrequencyDelayLine.h"
#include "util/Partitioning.h"
#include "util/SplitBlock.h"
