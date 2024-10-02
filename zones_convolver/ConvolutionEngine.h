#pragma once

#include "Convolver.h"

#include <juce_core/juce_core.h>
#include <juce_dsp/juce_dsp.h>
#include <optional>
#include <variant>

namespace zones
{
template <typename Commands, typename Visitor>
class ConvolutionVisitorQueue
{
public:
    explicit ConvolutionVisitorQueue (Visitor & visitor)
        : visitor_ (visitor)
    {
    }

    void Service ()
    {
        fifo_.read (fifo_.getNumReady ())
            .forEach (
                [&] (int index)
                {
                    auto command = std::move (storage_ [static_cast<std::size_t> (index)]);
                    std::visit (visitor_, command);
                });
    }

    bool PushCommand (Commands & command)
    {
        if (fifo_.getFreeSpace () == 0)
            return false;

        const auto writer = fifo_.write (1);
        if (writer.blockSize1 != 0)
            storage_ [static_cast<std::size_t> (writer.startIndex1)] = std::move (command);
        else if (writer.blockSize2 != 0)
            storage_ [static_cast<std::size_t> (writer.startIndex2)] = std::move (command);

        return true;
    }

    bool HasPendingCommands () const
    {
        return fifo_.getNumReady () > 0;
    }

private:
    static constexpr int kMaxQueueSize = 400;

    Visitor & visitor_;

    juce::AbstractFifo fifo_ {kMaxQueueSize};
    std::vector<Commands> storage_ {kMaxQueueSize};
};

struct ConvolutionCommandQueue
{
    struct EngineReadyCommand
    {
        std::unique_ptr<Convolver> convolver;
    };

    using Commands = std::variant<EngineReadyCommand>;
    struct Visitor
    {
        virtual void operator() (EngineReadyCommand & engine_ready_command) = 0;
    };

    using VisitorQueue = ConvolutionVisitorQueue<Commands, Visitor>;
};

struct ConvolutionNotificationQueue
{
    struct DisposeEngineCommand
    {
        std::unique_ptr<Convolver> convolver;
    };

    using Commands = std::variant<DisposeEngineCommand>;
    struct Visitor
    {
        virtual void operator() (DisposeEngineCommand & dispose_engine_command) = 0;
    };

    using VisitorQueue = ConvolutionVisitorQueue<Commands, Visitor>;
};

class LoadIRJob : public juce::ThreadPoolJob
{
public:
    explicit LoadIRJob (juce::dsp::AudioBlock<const float> ir_block,
                        const juce::dsp::ProcessSpec & spec,
                        const Convolver::ConvolverSpec & convolver_spec,
                        ConvolutionCommandQueue::VisitorQueue & command_queue,
                        std::mutex & load_mutex,
                        std::function<void ()> on_loading_complete);
    ~LoadIRJob () override = default;
    JobStatus runJob () override;

private:
    juce::AudioBuffer<float> ir_buffer_;
    juce::dsp::ProcessSpec spec_;
    Convolver::ConvolverSpec convolver_spec_;
    ConvolutionCommandQueue::VisitorQueue & command_queue_;
    std::mutex & load_mutex_;
    std::function<void ()> on_loading_complete_;
};

struct ConvolutionEngineListener
{
    virtual void OnLoadingUpdated () = 0;
};

class ConvolutionEngine
    : public juce::dsp::ProcessorBase
    , public ConvolutionCommandQueue::Visitor
    , public ConvolutionNotificationQueue::Visitor
    , public juce::Thread
    , public juce::ListenerList<ConvolutionEngineListener>
{
public:
    explicit ConvolutionEngine (juce::ThreadPool & thread_pool);
    ~ConvolutionEngine () override;

    void run () override;
    void operator() (
        ConvolutionNotificationQueue::DisposeEngineCommand & dispose_engine_command) override;

    void LoadIR (juce::dsp::AudioBlock<const float> ir_block,
                 const Convolver::ConvolverSpec & convolver_spec);
    void Clear ();

    void operator() (ConvolutionCommandQueue::EngineReadyCommand & engine_ready_command) override;

    void prepare (const juce::dsp::ProcessSpec & spec) override;
    void process (const juce::dsp::ProcessContextReplacing<float> & replacing) override;
    void reset () override;

    [[nodiscard]] bool IsLoading () const;

private:
    void EmitLoadingEvent ();

    void BeginFade ();
    void ResetFade ();

    void SetFadeTime (int fade_time_samples);

    std::unique_ptr<Convolver> convolver_;
    std::unique_ptr<Convolver> fade_convolver_;
    std::unique_ptr<Convolver> pending_convolver_;

    juce::LinearSmoothedValue<float> smoothed_value_in_;
    juce::LinearSmoothedValue<float> smoothed_value_out_;

    juce::AudioBuffer<float> fade_buffer_;
    std::optional<juce::dsp::ProcessSpec> spec_ = std::nullopt;
    juce::ThreadPool & thread_pool_;
    ConvolutionCommandQueue::VisitorQueue command_queue_;
    ConvolutionNotificationQueue::VisitorQueue notification_queue_;
    Convolver::FadeStrategy current_fade_strategy_ = Convolver::FadeStrategy::kInOut;

    static constexpr float kLowerFadeTimeS = 0.1f;
    static constexpr float kUpperFadeTimeS = 4.0f;
    static constexpr float kFadeRatio = 0.14f;
    static constexpr float kMaxCrossFadeIrTimeS = 10.0f;
    static constexpr float kInOutFadeTimeS = 0.4f;

    int lower_fade_bound_;
    int upper_fade_bound_;
    int max_crossfade_ir_length_;
    int in_out_fade_length_;

    std::mutex load_mutex_;
    bool is_loading_ = false;
};
}