cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(home-network)

# TODO
# https://rix0r.nl/blog/2015/08/13/cmake-guide/

link_directories(vendor/lib)

enable_testing()
add_subdirectory(libhome_network)
add_subdirectory(test_publisher)
