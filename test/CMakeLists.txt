add_executable(unit_tests)
target_sources(unit_tests PRIVATE
        SplitBlockTests.cpp
        ComplexBufferTests.cpp
        FFTDecompositionTests.cpp
        DecompositionScheduleTests.cpp
)

target_link_libraries(unit_tests PRIVATE
        zones_convolver
        melatonin_audio_sparklines
        melatonin_test_helpers
        Catch2::Catch2WithMain
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

target_compile_definitions(unit_tests
        PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_compile_options(unit_tests
        INTERFACE
        -Wall
        -Wextra
        -Werror
        -Wpedantic
)

include(Catch)
catch_discover_tests(unit_tests)