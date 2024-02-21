#include "TimeDistributedNUPC.h"

TimeDistributedNUPC::TimeDistributedNUPC (juce::dsp::AudioBlock<const float> ir_buffer,
                                          const juce::dsp::ProcessSpec & process_spec)
{
    // Get partitioning scheme, ir_num_samples, block_size ->
    // vector<{
    //  partition_size (in B), num_partitions
    // }>
    //
    // Prepare UPC with 0th index of partition scheme -> unique_ptr??
    // For (plan_vector...)
    // calculate clearance -> getCleranceForIndex() ?
    // add a TDUPC to vector
    // init circ buffer --> getMaxCleranceForShedule(schedule)
}

void TimeDistributedNUPC::Process (const juce::dsp::ProcessContextReplacing<float> & replacing)
{
}

void TimeDistributedNUPC::Reset ()
{
    // clear circ buffer
    // call reset on UPC/TDUPC's
}
