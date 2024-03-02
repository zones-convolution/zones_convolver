#pragma once

#include "time_distributed/TimeDistributedNUPC.h"

#include <juce_core/juce_core.h>
#include <juce_dsp/juce_dsp.h>
#include <optional>
#include <variant>

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
        std::unique_ptr<TimeDistributedNUPC> convolver;
    };

    using Commands = std::variant<EngineReadyCommand>;
    struct Visitor
    {
        virtual void operator() (EngineReadyCommand & engine_ready_command) = 0;
    };

    using VisitorQueue = ConvolutionVisitorQueue<Commands, Visitor>;
};

class LoadIRJob : public juce::ThreadPoolJob
{
public:
    explicit LoadIRJob (juce::dsp::AudioBlock<const float> ir_block,
                        const juce::dsp::ProcessSpec & spec,
                        ConvolutionCommandQueue::VisitorQueue & command_queue);
    ~LoadIRJob () override = default;
    JobStatus runJob () override;

private:
    juce::AudioBuffer<float> ir_buffer_;
    juce::dsp::ProcessSpec spec_;
    ConvolutionCommandQueue::VisitorQueue & command_queue_;
};

class ConvolutionEngine
    : public juce::dsp::ProcessorBase
    , public ConvolutionCommandQueue::Visitor
{
public:
    struct EngineSpec
    {
    };

    void operator() (ConvolutionCommandQueue::EngineReadyCommand & engine_ready_command) override;

    explicit ConvolutionEngine (juce::ThreadPool & thread_pool);
    ~ConvolutionEngine () override = default;
    void prepare (const juce::dsp::ProcessSpec & spec) override;
    void process (const juce::dsp::ProcessContextReplacing<float> & replacing) override;
    void reset () override;

    void ConfigureEngine (const EngineSpec & engine_spec);
    void LoadIR (juce::dsp::AudioBlock<const float> ir_block);

private:
    float
    SmoothedValue (float value_to_smooth, float target, float smooth_time_in_seconds = 0.2f) const;

    std::unique_ptr<TimeDistributedNUPC> convolver_;
    std::unique_ptr<TimeDistributedNUPC> pending_convolver_;

    float activity_coefficient_ = 0.f;

    std::optional<juce::dsp::ProcessSpec> spec_ = std::nullopt;
    juce::ThreadPool & thread_pool_;
    ConvolutionCommandQueue::VisitorQueue command_queue_;
    EngineSpec engine_spec_;
    juce::AudioBuffer<float> ir_buffer_;
};