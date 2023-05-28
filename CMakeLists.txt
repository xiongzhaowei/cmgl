
cmake_minimum_required(VERSION 3.15.0)

project(cmgl LANGUAGES CXX)

get_filename_component(SRCROOT "${CMAKE_SOURCE_DIR}" ABSOLUTE)

add_subdirectory("examples")
add_subdirectory("src")
