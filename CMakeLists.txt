# Specify the minimum required version of CMake.
cmake_minimum_required(VERSION 3.5)

# Define the project name and specify the programming language.
project(EventProcessor LANGUAGES CXX)

# Set the C++ standard to version 23 and require its usage.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Check if the build type is Release or Debug
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native")    
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

add_subdirectory(apps)
