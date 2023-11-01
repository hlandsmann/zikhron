cmake_minimum_required(VERSION 3.19)
project(zikhron)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CPM_SOURCE_CACHE "${CMAKE_SOURCE_DIR}/build_modules")
include(cmake/CPM.cmake)
CPMAddPackage("gh:catchorg/Catch2@3.3.2")
CPMAddPackage("gh:facebook/folly@2023.07.24.00")
CPMAddPackage("gh:fmtlib/fmt#9.1.0")
CPMAddPackage("gh:gabime/spdlog@1.11.0")
CPMAddPackage("gh:hanickadot/compile-time-regular-expressions#v3.8")
CPMAddPackage("gh:microsoft/GSL@4.0.0")
CPMAddPackage("gh:nlohmann/json@3.11.2")
CPMAddPackage("gh:glfw/glfw#3.3.8")
include(FindPkgConfig)

option(use_iwyu ON)

if(use_iwyu)
    find_program(IWYU_PATH NAMES include-what-you-use iwyu)

    if(NOT IWYU_PATH)
        message(FATAL_ERROR "Could not find the program include-what-you-use")
    endif()

    set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE ${IWYU_PATH})
    set(CMAKE_C_INCLUDE_WHAT_YOU_USE ${IWYU_PATH})
endif(use_iwyu)

include(${CMAKE_SOURCE_DIR}/cmake/gen_cfg_json.cmake)
gen_cfg_json()
add_subdirectory(src)
add_subdirectory(third_party)
option(ENABLE_TESTING "Enable Test Builds" ON)

if(ENABLE_TESTING)
    enable_testing()
    message("Building tests..")
    add_subdirectory(test)
    add_subdirectory(imguitest)
endif()


