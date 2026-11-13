cmake_minimum_required(VERSION 3.22)

project(SvarogWider
    LANGUAGES C CXX
    VERSION 0.1.0
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "")

# Add JUCE from the vendored copy
add_subdirectory(lib/JUCE)

# Add the plugin target (defined in src/CMakeLists.txt)
add_subdirectory(src)