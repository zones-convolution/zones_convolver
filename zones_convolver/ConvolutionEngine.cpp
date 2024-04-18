#include "ConvolutionEngine.h"

namespace zones
{
LoadIRJob::LoadIRJob (juce::dsp::AudioBlock<const float> ir_block,
                      const juce::dsp::ProcessSpec & spec,
                      const Convolver::ConvolverSpec & convolver_spec,
                      ConvolutionCommandQueue::VisitorQueue & command_queue)
    : ThreadPoolJob ("load_ir_job")
    , spec_ (spec)
    , convolver_spec_ (convolver_spec)
    , command_queue_ (command_queue)
{
    ir_buffer_.setSize (static_cast<int> (ir_block.getNumChannels ()),
                        static_cast<int> (ir_block.getNumSamples ()));
    juce::dsp::AudioBlock<float> {ir_buffer_}.copyFrom (ir_block);
}

juce::ThreadPoolJob::JobStatus LoadIRJob::runJob ()
{
    auto convolver = std::make_unique<Convolver> (ir_buffer_, spec_, convolver_spec_);
    ConvolutionCommandQueue::Commands command =
        ConvolutionCommandQueue::EngineReadyCommand {.convolver = std::move (convolver)};
    command_queue_.PushCommand (command);
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
    stopThread (10);
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
    smoothed_value_.setTargetValue (0.f);
    pending_convolver_ = std::move (engine_ready_command.convolver);
}

void ConvolutionEngine::prepare (const juce::dsp::ProcessSpec & spec)
{
    static constexpr auto kSmoothingTime = 0.6f;
    smoothed_value_.reset (spec.sampleRate, kSmoothingTime);
    spec_ = spec;

    reset ();
}

void ConvolutionEngine::process (const juce::dsp::ProcessContextReplacing<float> & replacing)
{
    command_queue_.Service ();

    if (convolver_ != nullptr)
        convolver_->Process (replacing);

    auto output_block = replacing.getOutputBlock ();

    if (smoothed_value_.isSmoothing ())
    {
        output_block.multiplyBy (smoothed_value_);

        if (! smoothed_value_.isSmoothing () && pending_convolver_ != nullptr)
        {
            ConvolutionNotificationQueue::Commands notification =
                ConvolutionNotificationQueue::DisposeEngineCommand {.convolver =
                                                                        std::move (convolver_)};
            notification_queue_.PushCommand (notification);
            convolver_ = std::move (pending_convolver_);
            pending_convolver_ = nullptr;
            smoothed_value_.setTargetValue (1.f);
        }
    }
}

void ConvolutionEngine::reset ()
{
    smoothed_value_.setCurrentAndTargetValue (1.f);
    if (convolver_ != nullptr)
        convolver_->Reset ();
}

void ConvolutionEngine::LoadIR (juce::dsp::AudioBlock<const float> ir_block,
                                const Convolver::ConvolverSpec & convolver_spec)
{
    if (spec_ != std::nullopt)
        thread_pool_.addJob (new LoadIRJob (ir_block, *spec_, convolver_spec, command_queue_),
                             true);
}
}