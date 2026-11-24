cmake_minimum_required(VERSION 3.22)

project(SvarogWider
    LANGUAGES C CXX
    VERSION 0.1.0
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "")

add_subdirectory(lib/JUCE)

juce_add_binary_data(SvarogWider_Assets
    SOURCES
        src/Fonts/Inter_18pt-Bold.ttf
        src/Fonts/Inter_18pt-Regular.ttf
)

add_subdirectory(src)