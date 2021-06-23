# MIT License 
# Copyright (c) 2021-Today Kamil Rog
#

find_package (Boost COMPONENTS system filesystem unit_test_framework REQUIRED)
include_directories (${TEST_SOURCE_DIR}/src
                     ${Boost_INCLUDE_DIRS}
                     )
add_definitions (-DBOOST_TEST_DYN_LINK)

# Add executable
add_executable (FourierTransformsTest unit/FourierTransformsTest.cpp)
add_executable (IntegrationTest integration/IntegrationTests.cpp)

# Link Libraries
target_link_libraries (FourierTransformsTest
                      ofdmlib
                      fftw3
                      ${Boost_FILESYSTEM_LIBRARY}
                      ${Boost_SYSTEM_LIBRARY}
                      ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
)

# Link Libraries
target_link_libraries (IntegrationTest
                      ofdmlib
                      fftw3
                      ${Boost_FILESYSTEM_LIBRARY}
                      ${Boost_SYSTEM_LIBRARY}
                      ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
)

# Add Tests
add_test (NAME Fourier_Transforms_Test COMMAND FourierTransformsTest)
add_test (NAME Integration_Test COMMAND IntegrationTest)
