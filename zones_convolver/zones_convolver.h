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

#include "util/CircularBuffer.h"
#include "util/ComplexBuffer.h"
#include "util/FrequencyDelayLine.h"
#include "util/SplitBlock.h"