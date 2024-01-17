FetchContent_Declare(melatonin_audio_sparklines
        GIT_REPOSITORY https://github.com/sudara/melatonin_audio_sparklines.git
        GIT_TAG origin/main
        SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/melatonin_audio_sparklines)
FetchContent_MakeAvailable(melatonin_audio_sparklines)
juce_add_module(${melatonin_audio_sparklines_SOURCE_DIR})

FetchContent_Declare(melatonin_test_helpers
        GIT_REPOSITORY https://github.com/zones-convolution/melatonin_test_helpers.git
        GIT_TAG technical/juce-update
        SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/melatonin_test_helpers)
FetchContent_MakeAvailable(melatonin_test_helpers)