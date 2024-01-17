cmake_minimum_required(VERSION 3.20)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

project(zones_convolver VERSION 1.0.0 LANGUAGES CXX
        DESCRIPTION "Convolution engine used by the Zones platform"
        HOMEPAGE_URL "https://github.com/zones-convolution/zones_convolver")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)

if (zones_convolver_IS_TOP_LEVEL)
    include(FetchContent)

    include(AddJuce)
    include(AddCatch2)
    include(AddMelatonin)

    juce_add_module(zones_convolver)

    enable_testing()
    add_subdirectory(test)
else ()
    juce_add_module(zones_convolver)
endif ()