set(ctre-hpp ${CMAKE_CURRENT_BINARY_DIR}/include/ctre/single-header/ctre.hpp)
cmake_path(GET ctre-hpp PARENT_PATH ctre-dir)
file(MAKE_DIRECTORY ${ctre-dir})

add_custom_command(
    OUTPUT ${ctre-hpp}
    COMMAND ${CMAKE_COMMAND} -E copy
        ${ctre_SOURCE_DIR}/single-header/ctre.hpp
        ${ctre-hpp}
    DEPENDS
        ${ctre_SOURCE_DIR}/single-header/ctre.hpp
)
add_custom_target(ctre_hpp DEPENDS ${ctre-hpp})
set_source_files_properties(${ctre-hpp} PROPERTIES GENERATED 1)
add_library(ctre-tmp OBJECT ${ctre-hpp})
set_target_properties(ctre-tmp PROPERTIES LINKER_LANGUAGE CXX)
add_library(ctre-single-header INTERFACE)
add_library(ctre::single-header ALIAS ctre-single-header)
add_dependencies(ctre-tmp ctre_hpp)
add_dependencies(ctre-single-header ctre-tmp)
target_sources(ctre-single-header PUBLIC $<TARGET_OBJECTS:ctre-tmp>)

target_include_directories(ctre-single-header
    INTERFACE
        ${CMAKE_CURRENT_BINARY_DIR}/include
)
