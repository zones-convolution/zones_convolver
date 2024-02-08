#include "TimeDistributedUPC.h"

#include "util/FFT.h"

void TimeDistributedUPC::Prepare (const juce::dsp::ProcessSpec & spec,
                                  int partition_size_samples,
                                  juce::dsp::AudioBlock<float> ir_segment)
{
    spec_ = spec;
    partition_size_samples_ = partition_size_samples;

    auto num_blocks_in_partition = partition_size_samples_ / spec_.maximumBlockSize;

    num_phases_ = num_blocks_in_partition;
    phase_ = 0;

    auto maximal_decompositions = std::log2 (num_phases_) - 1;
    forward_decomposition_plan_ = DecompositionSchedule::CreateForwardDecompositionPlan (
        num_blocks_in_partition, num_phases_, maximal_decompositions);

    inverse_decomposition_plan_ = DecompositionSchedule::CreateInverseDecompositionPlan (
        num_blocks_in_partition, num_phases_, maximal_decompositions);

    fft_num_points_ = 2 * partition_size_samples_;
    stage_buffers_ = std::make_unique<StageBuffers> (fft_num_points_);

    PrepareFilterPartitions (ir_segment, partition_size_samples, spec);

    // fft schedule prepare
}

void TimeDistributedUPC::Process (const juce::dsp::ProcessContextReplacing<float> & replacing)
{
    if (phase_ == 0)
        stage_buffers_->PromoteStages ();

    auto output_block = replacing.getOutputBlock ();
    auto block_size = output_block.getNumSamples ();

    /**
     * FILL STAGE BUFFERS -> Move to function??
     */
    {
        auto stage_a =
            stage_buffers_->GetStage (StageBuffers::StageBuffer::kA)->GetWritePointer (0);
        for (auto sample_index = 0; sample_index < block_size; ++sample_index)
            stage_a [(phase_ * block_size) + sample_index] =
                std::complex<float> {output_block.getSample (0, sample_index), 0.f};
    }

    DecompositionSchedule::ExecuteForwardDecompositionPlan (
        forward_decomposition_plan_, *stage_buffers_, fft_num_points_, num_phases_, phase_);

    // Forward FFT

    // Convolve

    // Inverse FFT
    
    DecompositionSchedule::ExecuteInverseDecompositionPlan (
        forward_decomposition_plan_, *stage_buffers_, fft_num_points_, num_phases_, phase_);

    /**
     * FILL OUTPUT BUFFER -> Move to function??
     */
    {
        auto stage_c =
            stage_buffers_->GetStage (StageBuffers::StageBuffer::kC)->GetWritePointer (0);
        for (auto sample_index = 0; sample_index < block_size; ++sample_index)
            output_block.setSample (
                0, sample_index, stage_c [(phase_ * block_size) + sample_index].real ());
    }

    phase_ = (phase_ + 1) % num_phases_;
}

void TimeDistributedUPC::PrepareFilterPartitions (juce::dsp::AudioBlock<float> ir_segment,
                                                  int partition_size_samples,
                                                  const juce::dsp::ProcessSpec & spec)
{
    auto filter_size = ir_segment.getNumSamples ();
    auto num_partitions = static_cast<int> (
        std::ceil (static_cast<float> (filter_size) / static_cast<float> (partition_size_samples)));

    frequency_delay_line_ =
        std::make_unique<FrequencyDelayLine> (1, num_partitions, fft_num_points_);

    for (auto partition_index = 0; partition_index < num_partitions; ++partition_index)
    {
        auto filter_partition_block = ir_segment.getSubBlock (
            partition_index * partition_size_samples, partition_size_samples);
        auto filter_partition = ComplexBuffer (fft_num_points_, 1);
        filter_partition.Clear ();
        filter_partition.CopyFromAudioBlock (filter_partition_block);
        ForwardFFTUnordered (filter_partition.GetWritePointer (0), fft_num_points_);
        filter_partitions_.emplace_back (std::move (filter_partition));
    }
}
