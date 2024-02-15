#include "TimeDistributedUPC.h"

#include "util/FFT.h"

void TimeDistributedUPC::Prepare (const juce::dsp::ProcessSpec & spec,
                                  int partition_size_samples,
                                  juce::dsp::AudioBlock<float> ir_segment)
{
    spec_ = spec;
    partition_size_samples_ = partition_size_samples;

    auto num_blocks_in_partition = partition_size_samples_ / spec_.maximumBlockSize;

    num_decompositions_ = static_cast<int> (std::log2 (num_blocks_in_partition / 2));
    num_phases_ = num_blocks_in_partition;
    phase_ = 0;

    fft_num_points_ = 2 * partition_size_samples_;
    stage_buffers_ = std::make_unique<StageBuffers> (fft_num_points_);

    PrepareFilterPartitions (ir_segment, partition_size_samples, spec);
}

void TimeDistributedUPC::Process (const juce::dsp::ProcessContextReplacing<float> & replacing)
{
    if (phase_ == 0)
    {
        stage_buffers_->PromoteStages ();
        auto stage_a = stage_buffers_->GetStage (StageBuffers::StageBuffer::kA);
        stage_a->Clear ();
        auto & first_fdl_block = frequency_delay_line_->GetNextBlock ();
        first_fdl_block.Clear ();
    }

    auto output_block = replacing.getOutputBlock ();
    auto block_size = output_block.getNumSamples ();

    // Fill input buffer
    auto stage_a = stage_buffers_->GetStage (StageBuffers::StageBuffer::kA)->GetWritePointer (0);
    for (auto sample_index = 0; sample_index < block_size; ++sample_index)
        stage_a [(phase_ * block_size) + sample_index] =
            std::complex<float> {output_block.getSample (0, sample_index), 0.f};

    // Block Size 1024
    // 8192 Partition Size
    // 16384 FFT Size

    // Num Phases: 8

    DecompositionSchedule::ForwardDecompositionSchedule (
        num_decompositions_, fft_num_points_, num_phases_, stage_a, phase_);

    auto stage_b = stage_buffers_->GetStage (StageBuffers::StageBuffer::kB)->GetWritePointer (0);
    auto sub_fft_size = fft_num_points_ / (static_cast<int> (std::pow (2, num_decompositions_)));
    auto half_sub_fft_size = sub_fft_size / 2;
    auto offset = phase_ * half_sub_fft_size;

    if (phase_ % 2 == 0)
    {
        auto sub_fft_data = &stage_b [offset];
        ForwardFFTUnordered (sub_fft_data, sub_fft_size);

        auto first_fdl = frequency_delay_line_->GetBlockWithOffset (0).GetWritePointer (0);
        for (auto point_index = 0; point_index < sub_fft_size; ++point_index)
        {
            first_fdl [offset + point_index] = sub_fft_data [point_index];
            sub_fft_data [point_index] = {0.f, 0.f};
        }
    }

    for (auto fdl_index = 0; fdl_index < num_partitions_; ++fdl_index)
    {
        auto fdl_block = frequency_delay_line_->GetBlockWithOffset (fdl_index).GetReadPointer (0);
        auto filter = filter_partitions_ [fdl_index].GetReadPointer (0);

        for (auto point_index = 0; point_index < half_sub_fft_size; ++point_index)
        {
            auto absolute_point = offset + point_index;
            stage_b [absolute_point] += fdl_block [absolute_point] * filter [absolute_point];
        }
    }

    if (phase_ % 2 != 0)
    {
        auto sub_fft_data = &stage_b [((phase_ - 1) / 2) * sub_fft_size];
        InverseFFTUnordered (sub_fft_data, sub_fft_size);
    }

    auto stage_c = stage_buffers_->GetStage (StageBuffers::StageBuffer::kC)->GetWritePointer (0);
    DecompositionSchedule::InverseDecompositionSchedule (
        num_decompositions_, fft_num_points_, num_phases_, stage_c, phase_);

    // Fill output buffer
    for (auto sample_index = 0; sample_index < block_size; ++sample_index)
        output_block.setSample (
            0, sample_index, stage_c [(phase_ * block_size) + sample_index].real ());

    phase_ = (phase_ + 1) % num_phases_;
}

void TimeDistributedUPC::PrepareFilterPartitions (juce::dsp::AudioBlock<float> ir_segment,
                                                  int partition_size_samples,
                                                  const juce::dsp::ProcessSpec & spec)
{
    auto filter_size = ir_segment.getNumSamples ();
    num_partitions_ = static_cast<int> (
        std::ceil (static_cast<float> (filter_size) / static_cast<float> (partition_size_samples)));

    frequency_delay_line_ =
        std::make_unique<FrequencyDelayLine> (1, num_partitions_, fft_num_points_);

    for (auto partition_index = 0; partition_index < num_partitions_; ++partition_index)
    {
        auto offset = partition_index * partition_size_samples;
        auto filter_partition_block = ir_segment.getSubBlock (
            offset, std::min (partition_size_samples, static_cast<int> (filter_size - offset)));
        auto filter_partition = ComplexBuffer (fft_num_points_, 1);
        filter_partition.Clear ();
        filter_partition.CopyFromAudioBlock (filter_partition_block);
        ForwardFFTUnordered (filter_partition.GetWritePointer (0), fft_num_points_);
        filter_partitions_.emplace_back (std::move (filter_partition));
    }
}
