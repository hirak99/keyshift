cmake_minimum_required(VERSION 3.10)
project(KeyboardRemapper)

# Set the C++ standard and compiler flags
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")

# Specify the output directories for executables and libraries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

find_package(Catch2 3 REQUIRED)

# Add the executable for demo_send_keys
add_executable(demo_send_keys demo_send_keys.cpp)
add_executable(keyboard_remap keyboard_remap.cpp)

# Add the tests.
enable_testing()
add_executable(remap_operator_test remap_operator_test.cpp)
target_link_libraries(remap_operator_test PRIVATE Catch2::Catch2WithMain)
add_test(NAME remap_operator_test COMMAND remap_operator_test)

