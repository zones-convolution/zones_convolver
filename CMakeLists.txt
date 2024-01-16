cmake_minimum_required(VERSION 3.20)

project(zones_convolver VERSION 1.0.0 LANGUAGES CXX
        DESCRIPTION "Convolution engine used by the Zones platform"
        HOMEPAGE_URL "https://github.com/zones-convolution/zones_convolver")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)

function(FetchJuce)
    FetchContent_Declare(JUCE
            GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
            GIT_TAG origin/master
            GIT_SHALLOW TRUE
            GIT_PROGRESS TRUE
            FIND_PACKAGE_ARGS 7.0.5)
    FetchContent_MakeAvailable(JUCE)
endfunction()

function(FetchCatch2)
    FetchContent_Declare(
            Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2.git
            GIT_TAG v3.3.2)
    FetchContent_MakeAvailable(Catch2)
endfunction()

function(FetchMelatoninSparklines)
    FetchContent_Declare(melatonin_audio_sparklines
            GIT_REPOSITORY https://github.com/sudara/melatonin_audio_sparklines.git
            GIT_TAG origin/main
            SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/melatonin_audio_sparklines)
    FetchContent_MakeAvailable(melatonin_audio_sparklines)
    juce_add_module(${melatonin_audio_sparklines_SOURCE_DIR})
endfunction()

function(FetchMelatoninTestHelpers)
    FetchContent_Declare(melatonin_test_helpers
            GIT_REPOSITORY https://github.com/zones-convolution/melatonin_test_helpers.git
            GIT_TAG technical/juce-update
            SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/melatonin_test_helpers)
    FetchContent_MakeAvailable(melatonin_test_helpers)
endfunction()

function(SetupTests)
    enable_testing()
    add_executable(zones_convolver_tests)
    target_sources(zones_convolver_tests PRIVATE
            tests/ConvolverUtilitiesTests.cpp
            tests/ComplexBufferTests.cpp
    )

    # Avoid compiling external dependencies with warning flags, probably a better way of doing this...
    add_library(test_dependencies INTERFACE)
    target_link_libraries(test_dependencies
            INTERFACE
            melatonin_audio_sparklines
            melatonin_test_helpers
            Catch2::Catch2WithMain)

    target_link_libraries(zones_convolver_tests PRIVATE
            zones_convolver
            test_dependencies
            juce::juce_recommended_config_flags
            juce::juce_recommended_lto_flags
            juce::juce_recommended_warning_flags
    )

    target_compile_definitions(zones_convolver_tests
            PUBLIC
            JUCE_WEB_BROWSER=0
            JUCE_VST3_CAN_REPLACE_VST2=0
    )

    target_compile_options(zones_convolver_tests
            INTERFACE
            -Wall
            -Wextra
            -Werror
            -Wpedantic
    )

    include(${Catch2_SOURCE_DIR}/extras/Catch.cmake)
    catch_discover_tests(zones_convolver_tests)
endfunction()

if (zones_convolver_IS_TOP_LEVEL)
    include(FetchContent)

    FetchJuce()
    FetchCatch2()
    FetchMelatoninSparklines()
    FetchMelatoninTestHelpers()

    juce_add_module(zones_convolver)
    SetupTests()
else ()
    juce_add_module(zones_convolver)
endif ()