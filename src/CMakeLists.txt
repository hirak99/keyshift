cmake_minimum_required(VERSION 3.30)
project(KeyShift)

# Set the C++ standard and compiler flags
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
# Release options.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -finline-functions -funroll-loops")

find_package(Boost)
include_directories(${Boost_INCLUDE_DIRS})

# Specify the output directories for executables and libraries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

find_package(Catch2 3 REQUIRED)

# Get the Git commit ID
execute_process(
    COMMAND git rev-parse HEAD
    OUTPUT_VARIABLE GIT_COMMIT_ID
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
    COMMAND git status --porcelain
    OUTPUT_VARIABLE GIT_STATUS
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
if (GIT_STATUS)
    set(GIT_COMMIT_ID "${GIT_COMMIT_ID}-dirty")
endif()
# Create the version header file
configure_file(
    ${CMAKE_SOURCE_DIR}/version.h.in
    ${CMAKE_BINARY_DIR}/version.h
)
# Needed to include the version.h.
include_directories(${CMAKE_BINARY_DIR})

add_executable(demo_send_keys demo_send_keys.cpp)
add_executable(keyshift os_level_mutex.cpp argparse.cpp config_parser.cpp keyshift.cpp remap_operator.cpp keycode_lookup.cpp)
# target_link_libraries(keyshift ${Boost_LIBRARIES})
# Strip debugging info.
set_target_properties(keyshift PROPERTIES LINK_FLAGS "-Wl,--gc-sections -Wl,--strip-all")

# How to profile -
# 1. Run `./profile && { gprof profile | less }`
# 2. Scroll down to "Call graph".
# 3. Check the correct function, e.g. Remapper::Process.
#   - index should match with function number, e.g. index [2] Remapper::Process [2],
#   - check seconds under children, which is total time including this function,
#   - divide by number of times called.
add_executable(profile profile.cpp config_parser.cpp remap_operator.cpp keycode_lookup.cpp)
set_target_properties(profile PROPERTIES COMPILE_FLAGS "-pg" LINK_FLAGS "-pg")
# target_link_libraries(profile ${Boost_LIBRARIES})

# Add the tests.
enable_testing()
add_executable(remap_operator_test remap_operator_test.cpp remap_operator.cpp keycode_lookup.cpp)
target_link_libraries(remap_operator_test PRIVATE Catch2::Catch2WithMain)
add_test(NAME remap_operator_test COMMAND remap_operator_test)

add_executable(config_parser_test config_parser_test.cpp config_parser.cpp remap_operator.cpp keycode_lookup.cpp)
target_link_libraries(config_parser_test PRIVATE Catch2::Catch2WithMain)
add_test(NAME config_parser_test COMMAND config_parser_test)
