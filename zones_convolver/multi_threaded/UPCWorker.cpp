#include "UPCWorker.h"

namespace zones
{

UPCWorker::UPCWorker (const juce::dsp::ProcessSpec & spec,
                      int partition_size_blocks,
                      juce::dsp::AudioBlock<const float> ir_segment)
    : partition_size_samples_ (partition_size_blocks * static_cast<int> (spec.maximumBlockSize))
    , upc_ (juce::dsp::ProcessSpec {.sampleRate = spec.sampleRate,
                                    .maximumBlockSize =
                                        static_cast<std::uint32_t> (partition_size_samples_),
                                    .numChannels = spec.numChannels},
            ir_segment)
{
    const auto block_size = static_cast<int> (spec.maximumBlockSize);
    const auto num_channels = static_cast<int> (spec.numChannels);
    const auto ir_num_channels = static_cast<int> (ir_segment.getNumChannels ());

    // must be at least two because FFT and IFFT are performed on separate phases
    jassert (partition_size_blocks > 1);

    scratch_buffer_.setSize (num_channels, block_size);
    scratch_buffer_.clear ();

    for (auto & buffer : double_buffers_)
    {
        buffer.setSize (num_channels, partition_size_samples_);
        buffer.clear ();
    }

    auto fft_num_points = 2 * partition_size_samples_;
    auto filter_size = static_cast<int> (ir_segment.getNumSamples ());
    auto num_partitions = GetNumPartitionsRequiredForSegment (partition_size_samples_, filter_size);

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
void UPCWorker::Process (const juce::dsp::ProcessContextReplacing<float> & replacing)
{
    // This will fail for offline render because there is nothing obstructing progress of process
    // meaning try_locks will be fail to be acquired

    // if fail flag -> write zeros

    auto scratch_block = juce::dsp::AudioBlock<float> (scratch_buffer_);
    // copy from swap buffer into scratch buffer
    // copy from replacing into swap buffer
    // copy from scratch buffer into replacing
    //  if we have enough samples (partition size)
    //  -> signal do convolution
    //
}
void UPCWorker::SignalDoConvolution ()
{
    const juce::GenericScopedTryLock<juce::SpinLock> scoped_try_lock (spin_lock_);
    if (scoped_try_lock.isLocked ())
    {
        // swap buffer
        worker_buffer_index_ = 1 - worker_buffer_index_;
    }
    else
    {
        // fail gracefully
        // set fail flag
        // clear input buffer
        // probably a counter until the fdl is fully realigned -> until then write zeros in the
        // output
    }
}
juce::dsp::AudioBlock<const float> UPCWorker::GetProcessedSamplesAndSwitchBuffer ()
{
}

void UPCWorker::Reset ()
{
    scratch_buffer_.clear ();
    upc_.Reset ();

    for (auto & buffer : double_buffers_)
        buffer.clear ();
}
}