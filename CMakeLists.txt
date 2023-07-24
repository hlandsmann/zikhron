cmake_minimum_required(VERSION 3.19)
project(zikhron)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
include(FindPkgConfig)

include(cmake/Conan.cmake)
option(skip_run_conan OFF)

run_conan()

# regenerate build system if conanfile.py changes
set_property(DIRECTORY
             APPEND
             PROPERTY CMAKE_CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/conanfile.py)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}")

find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)
find_package(ctre REQUIRED)
find_package(Microsoft.GSL REQUIRED)
find_package(nlohmann_json REQUIRED)

option(use_iwyu ON)

if(use_iwyu)
    find_program(IWYU_PATH NAMES include-what-you-use iwyu)

    if(NOT IWYU_PATH)
        message(FATAL_ERROR "Could not find the program include-what-you-use")
    endif()

    set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE ${IWYU_PATH})
    set(CMAKE_C_INCLUDE_WHAT_YOU_USE ${IWYU_PATH})
endif(use_iwyu)

add_subdirectory(src)

option(ENABLE_TESTING "Enable Test Builds" ON)

if(ENABLE_TESTING)
    enable_testing()
    message("Building tests..")
    add_subdirectory(test)
endif()

option(ENABLE_BENCHMARK "Enable Benchmarking" ON)

if(ENABLE_BENCHMARK)
    message("Building Benchmarks..")
    add_subdirectory(benchmark)
endif()
