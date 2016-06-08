###############################################################################
#
# Require 3.0 for INTERFACE projects

cmake_minimum_required (VERSION 3.0)

###############################################################################
#
# Find header files and put in separate target for IDE
#

file(GLOB_RECURSE MY_HDRS "*.hpp")
add_custom_target(resumable_dijkstra_headers SOURCES ${MY_HDRS})

###############################################################################
#
# Create an interface library for the header-only library
#

add_library(resumable_dijkstra INTERFACE)
target_include_directories(resumable_dijkstra INTERFACE "./")
target_link_libraries(resumable_dijkstra INTERFACE boost_interface)

###############################################################################
#
# Create an executable using the header only interface
#
add_executable(resumable_dijkstra_demo "demo.cpp")
target_link_libraries(resumable_dijkstra_demo PRIVATE resumable_dijkstra)

###############################################################################