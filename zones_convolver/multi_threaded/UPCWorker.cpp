#include "UPCWorker.h"

namespace zones
{

UPCWorker::UPCWorker (const juce::dsp::ProcessSpec & spec,
                      int partition_size_blocks,
                      juce::dsp::AudioBlock<const float> ir_segment)
{
    const auto block_size = static_cast<int> (spec.maximumBlockSize);
    const auto num_channels = static_cast<int> (spec.numChannels);
    const auto ir_num_channels = static_cast<int> (ir_segment.getNumChannels ());

    // must be at least two because FFT and IFFT are performed on separate phases
    jassert (partition_size_blocks > 1);

    auto partition_size_samples = partition_size_blocks * block_size;
    auto fft_num_points = 2 * partition_size_samples;
    auto filter_size = static_cast<int> (ir_segment.getNumSamples ());
    auto num_partitions = GetNumPartitionsRequiredForSegment (partition_size_samples, filter_size);

    //    for (auto partition_index = 0; partition_index < num_partitions; ++partition_index)
    //    {
    //        auto offset = partition_index * partition_size_samples;
    //        auto filter_partition_block = ir_segment.getSubBlock (
    //            offset, std::min (partition_size_samples, filter_size - offset));
    //
    //        auto filter_partition = ComplexBuffer (fft_num_points, ir_num_channels);
    //        filter_partition.Clear ();
    //        filter_partition.CopyFromAudioBlock (filter_partition_block);
    //
    //        for (auto channel_index = 0; channel_index < ir_num_channels; ++channel_index)
    //            ForwardFFTUnordered (filter_partition.GetWritePointer (channel_index),
    //            fft_num_points);
    //
    //        filter_partitions_.emplace_back (std::move (filter_partition));
    //    }
}

void UPCWorker::Reset ()
{
}
}