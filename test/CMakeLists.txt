# MIT License 
# Copyright (c) 2021-Today Kamil Rog
#

find_package (Boost COMPONENTS system filesystem unit_test_framework REQUIRED)
include_directories (${TEST_SOURCE_DIR}/src
                     ${Boost_INCLUDE_DIRS}
                     )
add_definitions (-DBOOST_TEST_DYN_LINK)

# Add executable

# Unit Tests
add_executable (FourierTransformsTest unit/FourierTransformsTest.cpp)
add_executable (BandPassModulatorTest unit/BandPassModulatorTest.cpp)  
add_executable (CyclicPrefixTest unit/CyclicPrefixTest.cpp) 

# Integration Tests
add_executable (IntegrationTest integration/IntegrationTests.cpp)

# Link libraries to unit tests
target_link_libraries (FourierTransformsTest
                      ofdmlib
                      fftw3
                      ${Boost_FILESYSTEM_LIBRARY}
                      ${Boost_SYSTEM_LIBRARY}
                      ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
)


target_link_libraries (BandPassModulatorTest
                      ofdmlib
                      fftw3
                      ${Boost_FILESYSTEM_LIBRARY}
                      ${Boost_SYSTEM_LIBRARY}
                      ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
)

find_package(Boost 1.40.0 COMPONENTS filesystem system iostreams REQUIRED)

target_link_libraries (CyclicPrefixTest
                      ofdmlib
                      fftw3
                      ${Boost_FILESYSTEM_LIBRARY}
                      ${Boost_SYSTEM_LIBRARY}
                      ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
                      ${Boost_LIBRARIES}
)


# Link libraries to integration tests
target_link_libraries (IntegrationTest
                      ofdmlib
                      fftw3
                      ${Boost_FILESYSTEM_LIBRARY}
                      ${Boost_SYSTEM_LIBRARY}
                      ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
)


# Add unit tests
add_test (NAME Fourier_Transforms_Test COMMAND FourierTransformsTest)
add_test (NAME Band_Pass_Modulator_Test COMMAND BandPassModulatorTest)
add_test (NAME Cyclic_Prefix_Test COMMAND CyclicPrefixTest)


# Add integration tests
add_test (NAME Integration_Test COMMAND IntegrationTest)
