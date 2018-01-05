cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(test-publisher VERSION 0.1 LANGUAGES C)

set(
        PROJECT_ROOT
        ${CMAKE_CURRENT_SOURCE_DIR}/../..)

#enable_testing()

add_executable(
        test-publisher
        src/main.c)

set_property(
        TARGET
        test-publisher
        PROPERTY
        C_STANDARD
        99)

target_compile_options(
        test-publisher
        PRIVATE
        -Wall
        -Wextra
        -Wno-unused-parameter
        -Wwrite-strings
        #-Wstrict-prototypes
        -Wmissing-include-dirs
        -Werror)

link_directories(
        ${PROJECT_ROOT}/src/vendor/lib)

include_directories(
        ${PROJECT_ROOT}/src/vendor/include
        ${PROJECT_ROOT}/src/vendor/include/rti_me)

target_link_libraries(
        test-publisher
        -static-libgcc
        -static)

target_link_libraries(
        test-publisher
        home_network
        pthread
        rt)

#target_include_directories(
#        test-publisher
#        PRIVATE
#        include)

# this makes the project importable from the build directory
export(
        TARGETS
        test-publisher
        FILE
        TestPublisherConfig.cmake)
