cmake_minimum_required(VERSION 3.30)
project(KeyboardRemapper)

# Set the C++ standard and compiler flags
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")

find_package(Boost REQUIRED COMPONENTS program_options)
include_directories(${Boost_INCLUDE_DIRS})

# Specify the output directories for executables and libraries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

find_package(Catch2 3 REQUIRED)

add_executable(demo_send_keys demo_send_keys.cpp)
add_executable(keyboard_remap config_parser.cpp keyboard_remap.cpp remap_operator.cpp keycode_lookup.cpp)
target_link_libraries(keyboard_remap ${Boost_LIBRARIES})

# Add the tests.
enable_testing()
add_executable(remap_operator_test remap_operator_test.cpp remap_operator.cpp keycode_lookup.cpp)
target_link_libraries(remap_operator_test PRIVATE Catch2::Catch2WithMain)
add_test(NAME remap_operator_test COMMAND remap_operator_test)

add_executable(config_parser_test config_parser_test.cpp config_parser.cpp remap_operator.cpp keycode_lookup.cpp)
target_link_libraries(config_parser_test PRIVATE Catch2::Catch2WithMain)
add_test(NAME config_parser_test COMMAND config_parser_test)
