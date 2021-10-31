cmake_minimum_required(VERSION 3.19)
project(zikhron)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include(FindPkgConfig)

add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/cppcoro)
find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)

find_program(IWYU_PATH NAMES include-what-you-use iwyu)
if(NOT IWYU_PATH)
  message(FATAL_ERROR "Could not find the program include-what-you-use")
endif()
set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE ${IWYU_PATH})
set(CMAKE_C_INCLUDE_WHAT_YOU_USE ${IWYU_PATH})

add_subdirectory(src)
add_executable(zikhron)
set_property(
  TARGET zikhron
  PROPERTY CXX_INCLUDE_WHAT_YOU_USE ${iwyu_path}
)
target_sources(zikhron
    PRIVATE
        src/zikhron.cpp
)
target_link_libraries(zikhron
    PRIVATE
        ${ICU_LIBRARIES}
        gui::gui
        gsl
        tcmalloc
        profiler
        pthread
)
target_compile_features(zikhron PRIVATE cxx_std_20)
target_link_options(zikhron PRIVATE LINKER:--gc-sections,--relax)
