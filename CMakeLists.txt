cmake_minimum_required(VERSION 3.19)
project(zikhron)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
include(FindPkgConfig)

include(cmake/Conan.cmake)
option(skip_run_conan OFF)
if (NOT skip_run_conan)
    run_conan()
    set(skip_run_conan ON CACHE BOOL "Run only once")
endif (NOT skip_run_conan)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}")

find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)
find_package(ctre REQUIRED)
find_package(Microsoft.GSL REQUIRED)
add_subdirectory(third_party/cppcoro)

option(use_iwyu OFF)
if (use_iwyu)
    find_program(IWYU_PATH NAMES include-what-you-use iwyu)
    if(NOT IWYU_PATH)
    message(FATAL_ERROR "Could not find the program include-what-you-use")
    endif()
    set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE ${IWYU_PATH})
    set(CMAKE_C_INCLUDE_WHAT_YOU_USE ${IWYU_PATH})
endif (use_iwyu)

add_subdirectory(src)
add_executable(zikhron)

target_sources(zikhron
    PRIVATE
        src/zikhron.cpp
)
target_link_libraries(zikhron
    PRIVATE
        zikhron::gui
)
target_compile_features(zikhron PRIVATE cxx_std_20)
target_link_options(zikhron PRIVATE LINKER:--gc-sections,--relax)

option(ENABLE_TESTING "Enable Test Builds" ON)
if(ENABLE_TESTING)
    enable_testing()
    message("Building tests..")
    add_subdirectory(test)
endif()
