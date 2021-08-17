# MIT License 
# Copyright (c) 2021-Today Kamil Rog
#
# Main CMake Project file


# Specify minimum CMake version
cmake_minimum_required(VERSION 3.10)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O3")
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Enable debug mode, all warrnings and prevent compilation if
# any errors are encountered 
add_compile_options(-g -Wall -Werror)

# Set the project name and version
project(ofdmlib VERSION 0.1)

# Allow for static analysis options
include(cmake/StaticAnalyzers.cmake)
# Add CPM https://github.com/cpm-cmake/CPM.cmake
include(cmake/CPM.cmake)

# Set Direcotries
set(INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(BIN_DIR ${PROJECT_SOURCE_DIR}/bin)
set(LIB_DIR ${PROJECT_SOURCE_DIR}/lib)

# Include Direcotries
add_subdirectory(src)
add_subdirectory(docs)

# Enable Testing
enable_testing ()

# Add Test directory
add_subdirectory(test)

#add_test (NAME ofdmlibTest COMMAND Test)