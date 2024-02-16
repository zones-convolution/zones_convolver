#pragma once

#include "SplitBlock.h"

#include <juce_dsp/juce_dsp.h>

class CircularBuffer
{
public:
    explicit CircularBuffer (juce::AudioBuffer<float> & buffer);
    SplitBlock GetNext (std::size_t advancement);
    SplitBlock GetNext (std::size_t delay, bool advance);

private:
    std::size_t head_position_ = 0;
    juce::AudioBuffer<float> & buffer_;
};