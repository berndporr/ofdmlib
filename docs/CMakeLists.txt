# MIT License 
# Copyright (c) 2021-Today Kamil Rog
#

# Indicate the documentation build as an option & set it on
option(BUILD_DOC "Build documentation" ON)

# Check if Doxygen is installed
find_package(Doxygen) # // This gave an error when repo was cloned on another machine https://gitlab.kitware.com/cmake/cmake/-/issues/18708
# If it is
if (DOXYGEN_FOUND)
    # Set I/O files
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.out)
    set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Doxygen/ )

    # Request to config the file
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    message("Doxygen build started")

    # Set option ALL which allows to build the documentation with the application
    add_custom_target( doc_doxygen ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        COMMENT "Generating documentation with Doxygen"
        VERBATIM )
# Doxygen not found
else (DOXYGEN_FOUND)
  message("Doxygen required to generate documentation!")
endif (DOXYGEN_FOUND)
