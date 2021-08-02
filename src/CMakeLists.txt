# MIT License 
# Copyright (c) 2021-Today Kamil Rog
#
# /src CMake Project file

set( CMAKE_VERBOSE_MAKEFILE on )
# Specify minimum CMake version
cmake_minimum_required(VERSION 3.10)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set the project name and version
project(ofdmlibDemo VERSION 0.1)

# Locate libraries and headers
find_package(Threads REQUIRED)

# Set Include Directories
set(INCLUDE_DIRS
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}/demo
   ${CMAKE_CURRENT_SOURCE_DIR}/utils
   ${CMAKE_CURRENT_SOURCE_DIR}/codec
   ${CMAKE_CURRENT_SOURCE_DIR}/codec/fft
   ${CMAKE_CURRENT_SOURCE_DIR}/codec/nyquist-modulator
   ${CMAKE_CURRENT_SOURCE_DIR}/codec/detector
   ${CMAKE_CURRENT_SOURCE_DIR}/codec/qam-modulator
)

# Set Source files
set(SOURCE_FILE    
   ${CMAKE_CURRENT_SOURCE_DIR}/codec/fft/ofdmfft.cpp 
   ${CMAKE_CURRENT_SOURCE_DIR}/codec/nyquist-modulator/nyquist-modulator.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/codec/detector/detector.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/utils/gnuplot-iostream.h
   ${CMAKE_CURRENT_SOURCE_DIR}/codec/ofdmcodec.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/demo/trx.cpp
)

# Create library
add_library(ofdmlib SHARED STATIC ${SOURCE_FILE} ) # what about static?

# Include directories so tests can find the files
target_include_directories(ofdmlib PUBLIC ${INCLUDE_DIRS})

install(TARGETS ofdmlib DESTINATION ${LIB_DIR})
#install(FILES encoder.h DESTINATION ${DIVISIBLE_INSTALL_INCLUDE_DIR})

FIND_LIBRARY(REQ_LIBRARIES NAMES libfftw3 rtaudio asound pthread) #jack?

# Add executable
add_executable(ofdmlibDemo ${CMAKE_CURRENT_SOURCE_DIR}/demo/demo.cpp)

# Link libraries to executable
target_link_libraries(ofdmlibDemo ofdmlib fftw3 rtaudio asound pthread ${CMAKE_THREAD_LIBS_INIT})

# TODO: FIND WHAT IS THE BEST WAY TO LINK fftw3 & HOW TO MAKE TESTS FIND THE FILES WITHOUT USING target_include_directories ON OFDMLIB