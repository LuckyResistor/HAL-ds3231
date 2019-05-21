# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.14)

# Set the project name
project(HAL-ds3231)

# Make sure we use the C++17 compiler standard
set(CMAKE_CXX_STANDARD 17)

# Create a static library.
file(GLOB SRC_FILES "*.cpp")
add_library(HAL-ds3231 ${SRC_FILES})

