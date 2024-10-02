#include "ConvolutionEngine.h"

#include <juce_events/juce_events.h>

namespace zones
{
LoadIRJob::LoadIRJob (juce::dsp::AudioBlock<const float> ir_block,
                      const juce::dsp::ProcessSpec & spec,
                      const Convolver::ConvolverSpec & convolver_spec,
                      ConvolutionCommandQueue::VisitorQueue & command_queue,
                      std::mutex & load_mutex,
                      std::function<void ()> on_loading_complete)
    : ThreadPoolJob ("load_ir_job")
    , spec_ (spec)
    , convolver_spec_ (convolver_spec)
    , command_queue_ (command_queue)
    , load_mutex_ (load_mutex)
    , on_loading_complete_ (on_loading_complete)
{
    ir_buffer_.setSize (static_cast<int> (ir_block.getNumChannels ()),
                        static_cast<int> (ir_block.getNumSamples ()));
    juce::dsp::AudioBlock<float> {ir_buffer_}.copyFrom (ir_block);
}

juce::ThreadPoolJob::JobStatus LoadIRJob::runJob ()
{
    auto convolver = std::make_unique<Convolver> (ir_buffer_, spec_, convolver_spec_);
    jassert (convolver != nullptr);

    ConvolutionCommandQueue::Commands command =
        ConvolutionCommandQueue::EngineReadyCommand {.convolver = std::move (convolver)};

    std::lock_guard guard {load_mutex_};

    if (! shouldExit ())
    {
        command_queue_.PushCommand (command);
        on_loading_complete_ ();
    }

    return jobHasFinished;
}

ConvolutionEngine::ConvolutionEngine (juce::ThreadPool & thread_pool)
    : juce::Thread ("convolution_engine_background")
    , thread_pool_ (thread_pool)
    , command_queue_ (*this)
    , notification_queue_ (*this)
{
    startThread ();
}

ConvolutionEngine::~ConvolutionEngine ()
{
    stopThread (100);
}

void ConvolutionEngine::run ()
{
    while (! threadShouldExit ())
    {
        notification_queue_.Service ();
        sleep (10);
    }
}

void ConvolutionEngine::operator() (
    ConvolutionNotificationQueue::DisposeEngineCommand & dispose_engine_command)
{
    dispose_engine_command.convolver.reset ();
}

void ConvolutionEngine::operator() (
    ConvolutionCommandQueue::EngineReadyCommand & engine_ready_command)
{
    if (convolver_ == nullptr)
    {
        convolver_ = std::move (engine_ready_command.convolver);
        return;
    }

    if (pending_convolver_ != nullptr)
    {
        ConvolutionNotificationQueue::Commands notification =
            ConvolutionNotificationQueue::DisposeEngineCommand {.convolver =
                                                                    std::move (pending_convolver_)};
        notification_queue_.PushCommand (notification);
    }

    if (fade_convolver_ != nullptr)
    {
        pending_convolver_ = std::move (engine_ready_command.convolver);
    }
    else
    {
        fade_convolver_ = std::move (engine_ready_command.convolver);
        BeginFade ();
    }
}

void ConvolutionEngine::BeginFade ()
{
    auto ir_size = fade_convolver_->GetNumSamples ();

    auto & convolver_spec = fade_convolver_->GetConvolverSpec ();
    current_fade_strategy_ = convolver_spec.fade_strategy;

    if (ir_size >= max_crossfade_ir_length_)
        current_fade_strategy_ = Convolver::FadeStrategy::kInOut;

    if (current_fade_strategy_ == Convolver::FadeStrategy::kCrossfade)
    {
        auto fade_samples =
            std::clamp (static_cast<int> (std::ceil (kFadeRatio * static_cast<float> (ir_size))),
                        lower_fade_bound_,
                        upper_fade_bound_);

        SetFadeTime (fade_samples);
    }
    else
    {
        SetFadeTime (in_out_fade_length_);
    }

    smoothed_value_in_.setTargetValue (1.f);
    smoothed_value_out_.setTargetValue (0.f);
}

void ConvolutionEngine::ResetFade ()
{
    smoothed_value_in_.setCurrentAndTargetValue (0.0f);
    smoothed_value_out_.setCurrentAndTargetValue (1.0f);
}

void ConvolutionEngine::prepare (const juce::dsp::ProcessSpec & spec)
{
    // Should we be resetting Convolvers here to the newest spec ?? - ie rebuilding scheme for new
    // block size etc... + channels etc?

    spec_ = spec;

    lower_fade_bound_ =
        static_cast<int> (std::ceil (kLowerFadeTimeS * static_cast<float> (spec.sampleRate)));
    upper_fade_bound_ =
        static_cast<int> (std::ceil (kUpperFadeTimeS * static_cast<float> (spec.sampleRate)));

    max_crossfade_ir_length_ =
        static_cast<int> (std::ceil (kMaxCrossFadeIrTimeS * static_cast<float> (spec.sampleRate)));

    in_out_fade_length_ =
        static_cast<int> (std::ceil (kInOutFadeTimeS * static_cast<float> (spec.sampleRate)));

    fade_buffer_.setSize (spec.numChannels, spec.maximumBlockSize);

    reset ();
}

void ConvolutionEngine::process (const juce::dsp::ProcessContextReplacing<float> & replacing)
{
    command_queue_.Service ();

    if (fade_convolver_ != nullptr)
    {
        auto output_block = replacing.getOutputBlock ();
        if (current_fade_strategy_ == Convolver::FadeStrategy::kCrossfade)
        {
            // Process the cross-fade

            juce::dsp::AudioBlock<float> fade_block {fade_buffer_};
            fade_block.copyFrom (output_block);

            if (convolver_ != nullptr)
                convolver_->Process (replacing);

            fade_convolver_->Process (fade_block);

            output_block.multiplyBy (smoothed_value_out_);
            fade_block.multiplyBy (smoothed_value_in_);
            output_block.add (fade_block);
        }
        else
        {
            if (smoothed_value_out_.isSmoothing ())
            {
                if (convolver_ != nullptr)
                    convolver_->Process (replacing);

                output_block.multiplyBy (smoothed_value_out_);
            }
            else
            {
                fade_convolver_->Process (replacing);
                output_block.multiplyBy (smoothed_value_in_);
            }
        }

        if (! smoothed_value_in_.isSmoothing ())
        {
            // Fade has finished
            ConvolutionNotificationQueue::Commands dispose_command =
                ConvolutionNotificationQueue::DisposeEngineCommand {.convolver =
                                                                        std::move (convolver_)};
            notification_queue_.PushCommand (dispose_command);
            convolver_ = std::move (fade_convolver_);

            ResetFade ();

            if (pending_convolver_ != nullptr)
            {
                fade_convolver_ = std::move (pending_convolver_);
                pending_convolver_ = nullptr;
                BeginFade ();
            }
            else
            {
                fade_convolver_ = nullptr;
            }
        }
    }
    else if (convolver_ != nullptr)
    {
        convolver_->Process (replacing);
    }
}

void ConvolutionEngine::reset ()
{
    ResetFade ();
    is_loading_ = false;

    command_queue_.Service ();

    fade_buffer_.clear ();

    if (pending_convolver_ != nullptr)
        convolver_ = std::move (pending_convolver_);
    else if (fade_convolver_ != nullptr)
        convolver_ = std::move (fade_convolver_);

    if (convolver_ != nullptr)
        convolver_->Reset ();
}

void ConvolutionEngine::LoadIR (juce::dsp::AudioBlock<const float> ir_block,
                                const Convolver::ConvolverSpec & convolver_spec)
{
    std::lock_guard guard (
        load_mutex_); // Obtained to guarantee load order - probably not the best way to do this...

    thread_pool_.removeAllJobs (true, 0);

    if (spec_ != std::nullopt)
    {
        is_loading_ = true;
        EmitLoadingEvent ();

        thread_pool_.addJob (new LoadIRJob (ir_block,
                                            *spec_,
                                            convolver_spec,
                                            command_queue_,
                                            load_mutex_,
                                            [&]
                                            {
                                                is_loading_ = false;
                                                juce::MessageManager::callAsync (
                                                    [&] { EmitLoadingEvent (); });
                                            }),
                             true);
    }
}

void ConvolutionEngine::Clear ()
{
    reset ();
    convolver_ = nullptr;
    pending_convolver_ = nullptr;
}

bool ConvolutionEngine::IsLoading () const
{
    return is_loading_;
}

void ConvolutionEngine::EmitLoadingEvent ()
{
    call ([] (ConvolutionEngineListener & listener) { listener.OnLoadingUpdated (); });
}

void ConvolutionEngine::SetFadeTime (int fade_time_samples)
{
    smoothed_value_in_.reset (fade_time_samples);
    smoothed_value_out_.reset (fade_time_samples);
}

}