# CMake version 3.12 provides FindPython.
# CMake version 3.11 generalizes in which directories find_package() searches
# for XxxConfig.cmake or FindXxx.cmake under Lin, Win, etc.
# CMake version 3.9 provides OpenMP per language.
cmake_minimum_required(VERSION 3.12)

project(pico_tree
    LANGUAGES CXX
    VERSION 0.6.0
    DESCRIPTION "PicoTree is a C++ header only library with Python bindings for nearest neighbor searches and range searches using a KdTree."
    HOMEPAGE_URL "https://github.com/Jaybro/pico_tree")

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

################################################################################
# PicoTree, examples, unit tests and documentation.
################################################################################

set(PROJECT_PACKAGE_NAME "PicoTree")
add_subdirectory(src)

# Ignored when running cmake from setup.py using scikit-build.
if(NOT SKBUILD)
    option(BUILD_EXAMPLES "Enable the creation of PicoTree examples." ON)
    if(BUILD_EXAMPLES)
        add_subdirectory(examples)
    endif()

    include(CTest)
    find_package(GTest QUIET)
    if(BUILD_TESTING)
        if (GTEST_FOUND)
            # Tests are dependent on some common code.
            if(NOT TARGET pico_common)
                add_subdirectory(examples/pico_common)
            endif()

            enable_testing()
            add_subdirectory(test/pico_tree)
            message(STATUS "GTest found. Building unit tests.")
        else()
            message(STATUS "GTest not found. Unit tests will not be build.")
        endif()
    endif()

    find_package(Doxygen QUIET)
    option(BUILD_DOCS "Build documentation with Doxygen." ON)
    if(BUILD_DOCS)
        if(DOXYGEN_FOUND)
            set(DOC_TARGET_NAME ${PROJECT_NAME}_doc)
            # Hide the internal namespace from the documentation.
            # set(DOXYGEN_EXCLUDE_SYMBOLS "internal")

            doxygen_add_docs(
                ${DOC_TARGET_NAME}
                src/pico_tree)

            message(STATUS "Doxygen found. Documentation can be build as: make ${DOC_TARGET_NAME}")
        else()
            message(STATUS "Doxygen not found. Documentation cannot be build.")
        endif()
    endif()
endif()
