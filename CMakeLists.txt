cmake_minimum_required(VERSION 3.22)

project(SvarogWider
    LANGUAGES C CXX
    VERSION 0.1.0
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "")

# Copying the built plugins into the system plug-in folders is handy when
# building on a workstation, but it is undesirable on build machines (CI),
# where the build output is packaged and published instead.
option(SVAROG_WIDER_COPY_PLUGINS_AFTER_BUILD
       "Copy the built plugins into the system plug-in folders after building" ON)

# The sources contain UTF-8 characters (mostly in comments). Tell MSVC about
# this explicitly so it never falls back to the system code page.
if(MSVC)
    add_compile_options(/utf-8)
endif()

add_subdirectory(lib/JUCE)

juce_add_binary_data(SvarogWider_Assets
    SOURCES
        src/Fonts/Inter_18pt-Bold.ttf
        src/Fonts/Inter_18pt-Regular.ttf
)

add_subdirectory(src)