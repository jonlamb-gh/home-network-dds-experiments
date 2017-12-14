cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(libhome_network VERSION 0.1 LANGUAGES C)

enable_testing()

add_library(
        home_network
        src/home_network.c)

set_property(
        TARGET
        home_network
        PROPERTY
        C_STANDARD
        99)

target_include_directories(
        home_network
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        src)

# this makes the project importable from the build directory
export(
        TARGETS
        home_network
        FILE
        LibHomeNetworkConfig.cmake)

add_executable(
        example_test
        tests/example_test.c)

set_property(
        TARGET
        example_test
        PROPERTY
        C_STANDARD
        99)

target_link_libraries(
        example_test
        home_network)

add_test(
        example_test
        example_test)
