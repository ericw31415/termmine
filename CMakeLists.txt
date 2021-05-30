cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wshadow -pedantic" CACHE STRING
    "Flags used by the C compiler during all build types")
set(CMAKE_CXX_FLAGS_DEBUG "-g -DNDEBUG" CACHE STRING
    "Flags used by the CXX compiler during DEBUG builds.")
set(CMAKE_CXX_FLAGS_RELEASE "-O2" CACHE STRING
    "Flags used by the C compiler during RELEASE builds.")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-s" CACHE STRING
    "Flags used by the linker during RELEASE builds.")

project(termmine CXX)

if(CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_CONFIGURATION_TYPES Debug Release CACHE STRING
        "List of configuration types to build" FORCE)

    string(REPLACE ";" ", " configs "${CMAKE_CONFIGURATION_TYPES}")
    message(STATUS "Found configurations: ${configs}")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
message(STATUS "Building from ${CMAKE_SOURCE_DIR}")
message(STATUS "Generating buildsystem in ${CMAKE_BINARY_DIR}")

add_subdirectory(src)
