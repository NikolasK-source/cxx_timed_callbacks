#
# Copyright (C) 2022 Nikolas Koesling <nikolas@koesling.info>.
# This program is free software. You can redistribute it and/or modify it under the terms of the MIT License.
#

cmake_minimum_required(VERSION 3.24.0 FATAL_ERROR)

# ----------------------------------------------- User settings --------------------------------------------------------
# ======================================================================================================================

# project
project(CMAKE_Template_Lib LANGUAGES CXX VERSION 0.0.1)

set(Target "Template_Lib")        # library name
set(STANDARD 17)                  # C++ Standard
set(ARCHITECTURE "native")        # CPU architecture to optimize for (only relevant if OPTIMIZE_FOR_ARCHITECTURE is ON)

# options
option(BUILD_DOC "Build documentation" ON)
option(COMPILER_WARNINGS "Enable compiler warnings" ON)
option(ENABLE_MULTITHREADING "Link the default multithreading library for the current target system" OFF)
option(MAKE_32_BIT_BINARY "Compile as 32 bit application. No effect on 32 bit Systems" OFF)
option(OPENMP "enable openmp" OFF)
option(OPTIMIZE_DEBUG "apply optimizations also in debug mode" ON)
option(CLANG_FORMAT "use clang-format" ON)
option(CLANG_TIDY "use clang-tidy" ON)
option(CLANG_TIDY_NO_ERRORS "do not treat clang-tidy warnings as errors" ON)
option(OPTIMIZE_FOR_ARCHITECTURE "enable optimizations for specified architecture" OFF)
option(COMPILER_EXTENSIONS "enable compiler specific C++ extensions" OFF)
option(ENABLE_TEST "enable test builds" ON)

# ======================================================================================================================
# ======================================================================================================================


# ----------------------------------------------- Do not change --------------------------------------------------------
# ======================================================================================================================

set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

include(CTest)
include(ClangFormat.cmake)

if(CLANG_TIDY)
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        if (${CLANG_TIDY_NO_ERRORS})
            set (CLANG_TIDY_CONFIG_FILE ${CMAKE_SOURCE_DIR}/.clang-tidy-noerrors)
        else()
            set (CLANG_TIDY_CONFIG_FILE ${CMAKE_SOURCE_DIR}/.clang-tidy)
        endif()

        set(CMAKE_CXX_CLANG_TIDY
                clang-tidy
                -config-file=${CLANG_TIDY_CONFIG_FILE})
        message(STATUS "clang-tidy enabled: ${CLANG_TIDY_CONFIG_FILE}")
    else()
        message(WARNING "clang-tidy requested, but only available if clang is selected as compiler")
    endif()
endif()

add_library(${Target} STATIC)

get_target_property(Target_CLANG_TIDY ${Target} CXX_CLANG_TIDY)
message(STATUS "Target_CLANG_TIDY: ${Target_CLANG_TIDY}")

set(CMAKE_CXX_CLANG_TIDY "")  # disable clang tidy for included libraries targets

# set source and include directory
add_subdirectory(src)
add_subdirectory(include)
add_subdirectory(libs)
target_include_directories(${Target} PUBLIC include)



include(warnings.cmake)
include(define.cmake)
include(compileropts.cmake)

# force C++ Standard and disable/enable compiler specific extensions
set_target_properties(${Target} PROPERTIES
        CXX_STANDARD ${STANDARD}
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS ${COMPILER_EXTENSIONS}
    )

# Determine whether this is a standalone project or included by other projects
set(STANDALONE_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(STANDALONE_PROJECT ON)
endif()

if(ENABLE_TEST AND STANDALONE_PROJECT)
    add_subdirectory(test)
endif()

if (NOT STANDALONE_PROJECT)
    unset(COMPILER_WARNINGS)
endif()

# options that are valid for gcc and clang
function(commonopts)
    # more debugging information
    if(OPTIMIZE_DEBUG)
        SET(CMAKE_CXX_FLAGS_DEBUG "-g3 -O3")
    else()
        SET(CMAKE_CXX_FLAGS_DEBUG "-g3")
    endif()

    if(MAKE_32_BIT_BINARY)
        message(STATUS "Compiling as 32 bit binary.")
        target_compile_options(${Target} PUBLIC -m32)
    endif()

    if(ENABLE_MULTITHREADING AND OPENMP)
        message(STATUS "openmp enabled")
        target_compile_options(${Target} PUBLIC -fopenmp)
    endif()

    if(OPTIMIZE_FOR_ARCHITECTURE)
        message(STATUS "using architecture specific code generator: ${ARCHITECTURE}")
        target_compile_options(${Target} PUBLIC -march=${ARCHITECTURE})
    endif()
endfunction()



if(COMPILER_WARNINGS)
    message(STATUS "Compiler warnings enabled.")
else()
    message(STATUS "Compiler warnings disabled.")
endif()

if(ENABLE_MULTITHREADING)
    # required by threading lib (std::thread)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    target_link_libraries(${Target} PRIVATE Threads::Threads)
endif()



if(BUILD_DOC)
    # doxygen documentation (https://vicrucann.github.io/tutorials/quick-cmake-doxygen/)
    # check if Doxygen is installed
    find_package(Doxygen)
    if (DOXYGEN_FOUND)
        # set input and output files
        set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
        set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

        if(EXISTS ${DOXYGEN_IN})
            # request to configure the file
            configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
            message(STATUS "Doxygen configured")

            # note the option ALL which allows to build the docs together with the application
            add_custom_target( doc_doxygen ALL
                    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                    COMMENT "Generating API documentation with Doxygen"
                    VERBATIM )
            message(STATUS "Added target doc_doxygen")
        else()
            message(WARNING "doxygen documentation requested, but file ${DOXYGEN_IN} does not exist.")
        endif()
    else (DOXYGEN_FOUND)
        message(WARNING "Doxygen need to be installed and accessible to generate the doxygen documentation.")
    endif (DOXYGEN_FOUND)
endif()

# add clang format target
if(CLANG_FORMAT)
    set(CLANG_FORMAT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/.clang-format)

    if(EXISTS ${CLANG_FORMAT_FILE})
        include(ClangFormat.cmake)
        target_clangformat_setup(${Target})
        message(STATUS "Added clang format target(s)")
    else()
        message(WARNING "Clang format enabled, but file ${CLANG_FORMAT_FILE}  does not exist")
    endif()
endif()
