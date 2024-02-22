#include "UniformPartitionedConvolver.h"

// void UniformPartitionedConvolver::prepare (const juce::dsp::ProcessSpec & spec)
//{
//     process_spec_ = spec;
// }
//
// void UniformPartitionedConvolver::process (
//     const juce::dsp::ProcessContextReplacing<float> & replacing)
//{
//     if (filter_partitions_.empty ())
//         return;
//
//     auto input_block = replacing.getInputBlock ();
//     auto output_block = replacing.getOutputBlock ();
//     auto num_channels = input_block.getNumChannels ();
//     auto num_partitions = filter_partitions_.size ();
//     auto num_samples = input_block.getNumSamples ();
//
//     auto & first_fdl_block = frequency_delay_line_->GetNextBlock ();
//     first_fdl_block.Clear ();
//     auto continuous_fdl_block = first_fdl_block.GetContinuousBlock ();
//
//     circular_buffer_->GetNext (0).CopyFrom (input_block);
//     circular_buffer_->GetNext (input_block.getNumSamples ()).CopyTo (continuous_fdl_block);
//
//     for (auto channel_index = 0u; channel_index < num_channels; ++channel_index)
//         fft_->performRealOnlyForwardTransform (
//             continuous_fdl_block.getChannelPointer (channel_index), true);
//
//     auto & first_partition = filter_partitions_ [0];
//     convolved_output_->ComplexMultiplyFrom (first_fdl_block, first_partition);
//
//     for (auto partition_index = 1u; partition_index < num_partitions; ++partition_index)
//     {
//         auto & previous_fdl_block = frequency_delay_line_->GetBlockWithOffset (partition_index);
//         auto & partition = filter_partitions_ [partition_index];
//
//         convolved_output_->ComplexMultiplyAccumulateFrom (previous_fdl_block, partition);
//     }
//
//     auto continuous_output_block = convolved_output_->GetContinuousBlock ();
//     for (auto channel_index = 0u; channel_index < num_channels; ++channel_index)
//         fft_->performRealOnlyInverseTransform (
//             continuous_output_block.getChannelPointer (channel_index));
//
//     output_block.copyFrom (
//         continuous_output_block.getSubBlock (num_samples_to_discard_, num_samples));
// }
//
// void UniformPartitionedConvolver::reset ()
//{
// }
//
// void UniformPartitionedConvolver::LoadImpulseResponse (juce::dsp::AudioBlock<float> ir_block,
//                                                        double sample_rate)
//{
//     auto num_channels = process_spec_.numChannels;
//     auto ir_num_channels = ir_block.getNumChannels ();
//     auto block_size = process_spec_.maximumBlockSize;
//
//     fft_size_ = block_size * 2;
//     fft_size_ = juce::nextPowerOfTwo (fft_size_);
//
//     auto fft_order = std::log2 (fft_size_);
//     fft_ = std::make_unique<juce::dsp::FFT> (fft_order);
//
//     num_samples_to_discard_ = fft_size_ - block_size;
//
//     auto num_samples = ir_block.getNumSamples ();
//     auto num_partitions = static_cast<int> (
//         std::ceil (static_cast<float> (num_samples) / static_cast<float> (block_size)));
//
//     saved_inputs_.setSize (num_channels, fft_size_);
//     saved_inputs_.clear ();
//
//     filter_partitions_.clear ();
//     ComplexBuffer input_partition {fft_size_, num_channels};
//
//     for (auto partition_index = 0; partition_index < num_partitions; ++partition_index)
//     {
//         auto partition_offset = partition_index * block_size;
//         auto partition_length =
//             std::min (block_size, static_cast<uint> (num_samples) - partition_offset);
//         auto partition_block = ir_block.getSubBlock (partition_offset, partition_length);
//
//         juce::AudioBuffer<float> copy_partition_buffer {static_cast<int> (num_channels),
//                                                         static_cast<int> (partition_length)};
//         for (auto channel_index = 0; channel_index < num_channels; ++channel_index)
//         {
//             juce::dsp::AudioBlock<float> (copy_partition_buffer)
//                 .getSingleChannelBlock (channel_index)
//                 .copyFrom (partition_block.getSingleChannelBlock (channel_index %
//                 ir_num_channels));
//         }
//
//         input_partition.Clear ();
//         input_partition.CopyFromAudioBlock (copy_partition_buffer);
//
//         ComplexBuffer filter_partition {fft_size_, num_channels};
//         filter_partition.Clear ();
//
//         for (auto channel_index = 0u; channel_index < num_channels; ++channel_index)
//             fft_->perform (input_partition.GetReadPointer (channel_index),
//                            filter_partition.GetWritePointer (channel_index),
//                            false);
//
//         filter_partitions_.emplace_back (std::move (filter_partition));
//     }
//
//     frequency_delay_line_ =
//         std::make_unique<FrequencyDelayLine> (num_channels, num_partitions, fft_size_);
//
//     convolved_output_ = std::make_unique<ComplexBuffer> (fft_size_, num_channels);
//     convolved_output_->Clear ();
//
//     circular_buffer_ = std::make_unique<CircularBuffer> (saved_inputs_);
// }

UniformPartitionedConvolver::UniformPartitionedConvolver (
    const juce::dsp::ProcessSpec & spec,
    juce::dsp::AudioBlock<const float> ir_segment)
{
}

void UniformPartitionedConvolver::Process (
    const juce::dsp::ProcessContextReplacing<float> & replacing)
{
}

void UniformPartitionedConvolver::Reset ()
{
}
