cmake_minimum_required(VERSION 3.10)
project(MyProject)

# Set the C++ standard and compiler flags
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")

# Specify the output directories for executables and libraries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Add the executable for demo_send_keys
add_executable(demo_send_keys demo_send_keys.cpp)

# Add the executable for remap_operator_test
add_executable(remap_operator_test remap_operator_test.cpp)

# Add a custom target for 'test' that runs remap_operator_test
add_custom_target(test COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/remap_operator_test DEPENDS remap_operator_test)
