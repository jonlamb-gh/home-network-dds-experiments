cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(libhome_network VERSION 0.1 LANGUAGES C)

set(
        PROJECT_ROOT
        ${CMAKE_CURRENT_SOURCE_DIR}/../..)

enable_testing()

add_library(
        home_network
        STATIC
        src/hn_timer.c
        src/hn_core.c)

set_property(
        TARGET
        home_network
        PROPERTY
        C_STANDARD
        99)

target_compile_definitions(
        home_network
        PUBLIC
        -DRTI_UNIX
        -DRTI_LINUX
        -DRTI_POSIX_THREADS)

target_compile_options(
        home_network
        PRIVATE
        -Wall
        -Wextra
        -Wno-unused-parameter
        -Wwrite-strings
        #-Wstrict-prototypes
        -Wmissing-include-dirs
        -Werror)

file(
        GLOB_RECURSE
        IDL_FILES
        "${PROJECT_ROOT}/idl/*.idl")

list(
        APPEND
        IDL_SOURCES
        "")

list(
        APPEND
        IDL_HEADERS
        "")

# copy all of the top level IDL files
add_custom_command(
        OUTPUT idl_sources/common.idl
        COMMAND mkdir -p idl
        COMMAND find "${PROJECT_ROOT}/idl" -iname "*.idl" -exec cp {} idl/ \\\;
        MAIN_DEPENDENCY ${PROJECT_ROOT}/idl/common/common.idl)

# generate the C files for each IDL file
foreach(idl_file ${IDL_FILES})
    get_filename_component(name ${idl_file} NAME_WE)

    list(APPEND IDL_HEADERS "idl_sources/${name}.h")
    list(APPEND IDL_SOURCES "idl_sources/${name}.c")
    list(APPEND IDL_HEADERS "idl_sources/${name}Plugin.h")
    list(APPEND IDL_SOURCES "idl_sources/${name}Plugin.c")
    list(APPEND IDL_HEADERS "idl_sources/${name}Support.h")
    list(APPEND IDL_SOURCES "idl_sources/${name}Support.c")

    add_custom_command(
            OUTPUT "idl_sources/${name}.h"
            OUTPUT "idl_sources/${name}.c"
            OUTPUT "idl_sources/${name}Plugin.h"
            OUTPUT "idl_sources/${name}Plugin.c"
            OUTPUT "idl_sources/${name}Support.h"
            OUTPUT "idl_sources/${name}Support.c"
            COMMAND mkdir -p idl_sources
            COMMAND rtiddsgen -micro -language C -replace -d idl_sources -I idl "${idl_file}"
            MAIN_DEPENDENCY ${idl_file}
            DEPENDS idl_sources/common.idl)
endforeach()

# add the generated IDL source files
target_sources(
        home_network
        PRIVATE
        ${IDL_SOURCES})

link_directories(
        ${PROJECT_ROOT}/src/vendor/lib)

include_directories(
        ${PROJECT_ROOT}/src/vendor/include
        ${PROJECT_ROOT}/src/vendor/include/rti_me)

target_link_libraries(
        home_network
        -static-libgcc
        -static)

# TODO - fix static linking
target_link_libraries(
        home_network
        rti_mez
        dl
        nsl
        m
        pthread
        rt)

target_include_directories(
        home_network
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/idl_sources>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        src)

# this makes the project importable from the build directory
export(
        TARGETS
        home_network
        FILE
        LibHomeNetworkConfig.cmake)

# TODO - make this better
add_executable(
        example-test
        tests/example_test.c)

set_property(
        TARGET
        example-test
        PROPERTY
        C_STANDARD
        99)

target_link_libraries(
        example-test
        -static-libgcc
        -static)

target_link_libraries(
        example-test
        home_network)

add_test(
        example-test
        example-test)

# timer-test
add_executable(
        timer-test
        tests/timer_test.c)

set_property(
        TARGET
        timer-test
        PROPERTY
        C_STANDARD
        99)

target_link_libraries(
        timer-test
        -static-libgcc
        -static)

target_link_libraries(
        timer-test
        home_network
        pthread
        rt)

add_test(
        timer-test
        timer-test)
