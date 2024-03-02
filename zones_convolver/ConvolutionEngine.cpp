#include "ConvolutionEngine.h"

LoadIRJob::LoadIRJob (juce::dsp::AudioBlock<const float> ir_block,
                      const juce::dsp::ProcessSpec & spec,
                      ConvolutionCommandQueue::VisitorQueue & command_queue)
    : ThreadPoolJob ("load_ir_job")
    , spec_ (spec)
    , command_queue_ (command_queue)
{
    ir_buffer_.setSize (static_cast<int> (ir_block.getNumChannels ()),
                        static_cast<int> (ir_block.getNumSamples ()));
    juce::dsp::AudioBlock<float> {ir_buffer_}.copyFrom (ir_block);
}

juce::ThreadPoolJob::JobStatus LoadIRJob::runJob ()
{
    auto convolver = std::make_unique<TimeDistributedNUPC> (ir_buffer_, spec_);
    ConvolutionCommandQueue::Commands command =
        ConvolutionCommandQueue::EngineReadyCommand {.convolver = std::move (convolver)};
    command_queue_.PushCommand (command);
    return jobHasFinished;
}

void ConvolutionEngine::operator() (
    ConvolutionCommandQueue::EngineReadyCommand & engine_ready_command)
{
    pending_convolver_ = std::move (engine_ready_command.convolver);
}

ConvolutionEngine::ConvolutionEngine (juce::ThreadPool & thread_pool)
    : thread_pool_ (thread_pool)
    , command_queue_ (*this)
{
}

void ConvolutionEngine::prepare (const juce::dsp::ProcessSpec & spec)
{
    spec_ = spec;
}

float ConvolutionEngine::SmoothedValue (float value_to_smooth,
                                        float target,
                                        float smooth_time_in_seconds) const
{
    auto delta = 1.0f / (spec_->sampleRate * smooth_time_in_seconds);
    auto step = (target - value_to_smooth) * delta;
    return value_to_smooth + step;
}

void ConvolutionEngine::process (const juce::dsp::ProcessContextReplacing<float> & replacing)
{
    command_queue_.Service ();

    if (convolver_ != nullptr)
        convolver_->Process (replacing);

    auto output_block = replacing.getOutputBlock ();
    auto block_size = static_cast<int> (output_block.getNumSamples ());
    auto num_channels = static_cast<int> (output_block.getNumChannels ());

    auto current_fade_value = 0.0f;
    auto is_transitioning = pending_convolver_ != nullptr;

    static constexpr float kTransitionThreshold = 0.0001f;

    if (is_transitioning || (1.f - activity_coefficient_) > kTransitionThreshold)
    {
        for (auto channel_index = 0; channel_index < num_channels; ++channel_index)
        {
            current_fade_value = activity_coefficient_;

            auto channel = output_block.getChannelPointer (channel_index);
            for (auto sample_index = 0; sample_index < block_size; ++sample_index)
            {
                current_fade_value =
                    SmoothedValue (current_fade_value, is_transitioning ? 0.0f : 1.0f);
                channel [sample_index] *= current_fade_value;
            }
        }
    }

    activity_coefficient_ = current_fade_value;

    if (is_transitioning && activity_coefficient_ < kTransitionThreshold)
    {
        convolver_.reset(); // SCHEDULE REMOVE OLD CONVOLVER : TODO
        convolver_ = std::move (pending_convolver_);

        pending_convolver_.reset ();
    }
}

void ConvolutionEngine::reset ()
{
    if (convolver_ != nullptr)
        convolver_->Reset ();
}

void ConvolutionEngine::ConfigureEngine (const EngineSpec & engine_spec)
{
}

void ConvolutionEngine::LoadIR (juce::dsp::AudioBlock<const float> ir_block)
{
    if (spec_ != std::nullopt)
        thread_pool_.addJob (new LoadIRJob (ir_block, *spec_, command_queue_), true);
}
