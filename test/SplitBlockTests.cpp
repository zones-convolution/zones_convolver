#include <catch2/catch_test_macros.hpp>
#include <melatonin_test_helpers/melatonin_test_helpers.h>
#include <zones_convolver/util/SplitBlock.h>

SCENARIO ("split blocks can be cleared", "[SplitBlock]")
{
    SECTION ("can clear an empty block")
    {
        juce::dsp::AudioBlock<float> empty_block {};
        SplitBlock split_block {empty_block, empty_block};

        split_block.Clear ();
    }

    GIVEN ("a split block")
    {
        juce::AudioBuffer<float> buffer {1, 128};
        juce::dsp::AudioBlock<float> block {buffer};
        block.fill (1.0f);

        auto block_one = block.getSubBlock (0, 64);
        auto block_two = block.getSubBlock (64, 64);

        SplitBlock split_block {block_one, block_two};
        REQUIRE (melatonin::blockIsFilled (block));

        WHEN ("the block is cleared")
        {
            split_block.Clear ();
            THEN ("the full block is now empty")
            {
                REQUIRE (melatonin::blockIsEmpty (block));
            }
        }
    }
}

SCENARIO ("adding and copying into a Split Block", "[SplitBlock]")
{
    GIVEN ("an empty split block")
    {
        juce::AudioBuffer<float> buffer {1, 0};
        juce::dsp::AudioBlock<float> empty_block {buffer};

        SplitBlock split_block {empty_block, empty_block};
        juce::AudioBuffer<float> data_block {1, 128};

        //        THEN ("the block can be added to")
        //        {
        //            split_block.AddFrom (data_block);
        //        }

        //        THEN ("the block can be copied to")
        //        {
        //            split_block.CopyFrom (data_block);
        //        }
    }

    GIVEN ("a split block")
    {
        juce::AudioBuffer<float> buffer {1, 128};
        juce::dsp::AudioBlock<float> block {buffer};
        block.fill (0.0f);
        auto block_one = block.getSubBlock (0, 64);
        auto block_two = block.getSubBlock (64, 64);
        SplitBlock split_block {block_one, block_two};
        REQUIRE (melatonin::blockIsEmpty (block));

        WHEN ("adding from a shorter block")
        {
            juce::AudioBuffer<float> data {1, 64};
            juce::dsp::AudioBlock<float> data_block {buffer};
            data_block.fill (1.0f);
            split_block.AddFrom (data_block);

            THEN ("the split block still has empty samples")
            {
                REQUIRE (melatonin::blockIsFilled (block_one));
                REQUIRE (melatonin::blockIsEmpty (block_two));
            }
        }

        WHEN ("adding from a longer block")
        {
            juce::AudioBuffer<float> data {1, 128};
            juce::dsp::AudioBlock<float> data_block {buffer};
            data_block.fill (1.0f);
            split_block.AddFrom (data_block);

            THEN ("all the samples in the block are filled")
            {
                REQUIRE (melatonin::blockIsFilled (block));
            }
        }
    }
}

SCENARIO ("copying from a split block", "[SplitBlock]")
{
    GIVEN ("an empty split block")
    {
        juce::dsp::AudioBlock<float> empty_block {};
        SplitBlock split_block {empty_block, empty_block};

        juce::AudioBuffer<float> data_buffer {1, 128};
        juce::dsp::AudioBlock<float> data_block {data_buffer};
        data_block.fill (1.0f);

        WHEN ("data is copied")
        {
            split_block.CopyTo (data_block);

            THEN ("no samples are changed")
            {
                REQUIRE (melatonin::blockIsFilled (data_block));
            }
        }
    }

    SECTION ("split blocks of different lengths")
    {
        juce::AudioBuffer<float> data {1, 256};
        juce::dsp::AudioBlock<float> data_block {data};

        auto split_block_sizes = {32, 64, 128, 33, 77};
        for (auto block_one_size : split_block_sizes)
        {
            for (auto block_two_size : split_block_sizes)
            {
                juce::AudioBuffer<float> buffer_one {1, block_one_size};
                juce::AudioBuffer<float> buffer_two {1, block_two_size};

                juce::dsp::AudioBlock<float> block_one {buffer_one};
                juce::dsp::AudioBlock<float> block_two {buffer_two};

                block_one.fill (1.0f);
                block_two.fill (1.0f);

                SplitBlock split_block {block_one, block_two};
                data_block.fill (0.0f);

                split_block.CopyTo (data_block);
                THEN ("the correct length is copied")
                {
                    REQUIRE (melatonin::blockIsFilledUntil (data_block,
                                                            block_one_size + block_two_size));
                    REQUIRE (
                        melatonin::blockIsEmptyAfter (data_block, block_one_size + block_two_size));
                }
            }
        }
    }
}